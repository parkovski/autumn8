#include "a8/util/router.h"

#include <cassert>

using namespace a8;

Router::Router() {
  putString({});
  putString(":");
}

// FIXME: Route "/" does not add a transition to "" but does look up the
// empty string when routing.
auto Router::operator[](std::string_view path)
    -> std::function<callback_type> & {
  assert(!path.empty() && path[0] == '/');

  State *state = &_initial;
  size_t segstart = 0, segend;
  decltype(std::declval<Route>().params) params;
  while (segstart < path.size() - 1) {
    segend = path.find('/', ++segstart);
    auto seg = path.substr(segstart, segend - segstart);
    assert(!seg.empty());

    if (seg[0] == ':') {
      params.emplace_back(state->id, putString(seg.substr(1)));
      state = getOrPutTransition(state->id, ParamString);
    } else {
      state = getOrPutTransition(state->id, putString(seg));
    }
    segstart = segend;
  }
  assert(state->route == BadRoute && "Duplicate route");
  state->route = route_id_t(_routes.size());
  return _routes.emplace_back(Route{nullptr, std::move(params)}).callback;
}

bool Router::operator()(std::string_view path) const {
  ParamList phead{
    nullptr,
    0,
    {}
  };
  return route(path, _initial, &phead, &phead);
}

auto Router::getStringId(std::string_view str) const -> string_id_t {
  auto it = _stringToId.find(str);
  if (it == _stringToId.end()) {
    return BadString;
  }
  return it->second;
}

auto Router::putString(std::string_view str) -> string_id_t {
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

auto Router::getTransition(state_id_t stateId, string_id_t strId) const
    -> const State * {
  auto it = _transitions.find(std::make_pair(stateId, strId));
  if (it == _transitions.end()) {
    return nullptr;
  }
  return &it->second;
}

auto Router::getOrPutTransition(state_id_t stateId, string_id_t strId)
    -> State * {
  auto [it, inserted] = _transitions.emplace(std::make_pair(stateId, strId),
                                             State{_nextStateId});
  if (inserted) {
    ++_nextStateId;
  }
  return &it->second;
}

bool Router::route(std::string_view path, const State &state, ParamList *plist,
                   ParamList *phead) const {
  if (path.empty() || path == "/") {
    return callRoute(state.route, phead);
  }

  size_t segend = path.find('/', 1);
  auto seg = path.substr(1, segend - 1);
  assert(!seg.empty());
  path = path.substr(seg.size() + 1);
  string_id_t stringId = getStringId(seg);

  if (stringId != BadString) {
    if (auto const *nextState = getTransition(state.id, stringId)) {
      if (route(path, *nextState, plist, phead)) {
        return true;
      }
    }
  }

  if (auto const *nextState = getTransition(state.id, ParamString)) {
    ParamList pnext{nullptr, state.id, seg};
    plist->next = &pnext;
    return route(path, *nextState, &pnext, phead);
  }

  return false;
}

bool Router::callRoute(route_id_t routeId, ParamList *phead) const {
  if (routeId == BadRoute) {
    return false;
  }

  assert(_routes.size() > routeId);
  auto const &route = _routes[routeId];
  auto pdef = route.params.begin();
  ParamList *plist = phead->next;
  param_map_type pmap;
  while (plist) {
    assert(pdef != route.params.end());
    std::string_view paramName = _idToString[pdef->second].get();
    assert(plist->stateId == pdef->first && "Param position mismatch");
    pmap.emplace(paramName, plist->value);

    plist = plist->next;
    ++pdef;
  }
  assert(pdef == route.params.end());

  route.callback(pmap);
  return true;
}
