#include "a8/util/router.h"

#include <cassert>

using namespace a8;

RouterBase::RouterBase() {
  putString({});
  putString(":");
}

void RouterBase::fillParamMap(RouteParams &params,
                              const RouteBase &route) const {
  auto pname = route.params.begin();
  auto pval = params.paramv.begin();
  while (pname != route.params.end()) {
    assert(pval != params.paramv.end());
    auto sid = *pname;
    assert(sid != BadString);
    std::string_view paramName = _idToString[sid].get();
    params.paramm.emplace(paramName, *pval);

    ++pname;
    ++pval;
  }
}

bool RouterBase::insert(std::string_view path, route_id_t routeId,
                        RouteBase &route) {
  assert(!path.empty() && path[0] == '/');

  State *state = &_initial;
  size_t segstart = 0, segend;
  while (segstart < path.size() - 1) {
    segend = path.find('/', ++segstart);
    auto seg = path.substr(segstart, segend - segstart);
    assert(!seg.empty());

    if (seg[0] == ':') {
      if (seg == ":...") {
        assert(segend == std::string_view::npos
               && "Catch-all param (:...) must be the final route part");
        return putSelfTransition(state, routeId);
      } else {
        route.params.emplace_back(putString(seg.substr(1)));
        state = getOrPutTransition(state->id, ParamString);
      }
    } else {
      state = getOrPutTransition(state->id, putString(seg));
    }
    segstart = segend;
  }
  if (state->route != BadRoute) {
    return false; // Duplicate route
  }
  state->route = routeId;
  return true;
}

auto RouterBase::route(std::string_view path, RouteParams &params) const
    -> route_id_t {
  return routeStep(path, _initial, params);
}

auto RouterBase::routeStep(std::string_view path, const State &state,
                           RouteParams &params) const -> route_id_t {
  if (path.empty() || path == "/") {
    return state.route;
  }

  size_t segend = path.find('/', 1);
  auto seg = path.substr(1, segend - 1);
  assert(!seg.empty());
  string_id_t stringId = getStringId(seg);

  path = path.substr(seg.size() + 1);

  if (stringId != BadString) {
    if (auto const *nextState = getTransition(state.id, stringId)) {
      auto rid = routeStep(path, *nextState, params);
      if (rid != BadRoute) {
        return rid;
      }
    }
  }

  if (auto const *nextState = getTransition(state.id, ParamString)) {
    params.paramv.emplace_back(seg);
    return routeStep(path, *nextState, params);
  }

  return BadRoute;
}

auto RouterBase::getStringId(std::string_view str) const -> string_id_t {
  auto it = _stringToId.find(str);
  if (it == _stringToId.end()) {
    return BadString;
  }
  return it->second;
}

auto RouterBase::putString(std::string_view str) -> string_id_t {
  auto id = getStringId(str);
  if (id == BadString) {
    id = string_id_t(_idToString.size());
    auto &ownStr =
      _idToString.emplace_back(std::make_unique<char[]>(str.size() + 1));
    std::char_traits<char>::copy(ownStr.get(), str.data(), str.size());
    ownStr[str.size()] = 0;
    str = std::string_view{ownStr.get(), str.size()};
    _stringToId.emplace(str, id);
  }
  return id;
}

auto RouterBase::getTransition(state_id_t stateId, string_id_t strId) const
    -> const State * {
  auto it = _transitions.find(std::make_pair(stateId, strId));
  if (it == _transitions.end()) {
    return nullptr;
  }
  return &it->second;
}

auto RouterBase::getOrPutTransition(state_id_t stateId, string_id_t strId)
    -> State * {
  auto [it, inserted] = _transitions.emplace(std::make_pair(stateId, strId),
                                             State{_nextStateId});
  if (inserted) {
    ++_nextStateId;
  }
  return &it->second;
}

bool RouterBase::putSelfTransition(State *state, route_id_t routeId) {
  // This happens when you do something like this:
  // add route /foo
  // add route /foo/:...
  assert(state->route == BadRoute &&
         "Route already defined without catch-all");

  state->route = routeId;
  auto [it, inserted] =
    _transitions.emplace(std::make_pair(state->id, ParamString),
                         State{state->id, routeId});
  assert(inserted && "Catch-all route already defined as parameter");

  return true;
}
