#ifndef A8_UTIL_ROUTER_H_
#define A8_UTIL_ROUTER_H_

#include <boost/container/small_vector.hpp>
#include <boost/container_hash/hash.hpp>

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

namespace a8 {

class RouterBase {
public:
  // All route parameter values in order.
  using param_vec_type = std::vector<std::string_view>;
  // Named route parameters mapped to value.
  using param_map_type =
    std::unordered_map<std::string_view, std::string_view>;

  struct RouteParams {
  private:
    friend class RouterBase;
    param_vec_type paramv;
    param_map_type paramm;

  public:
    RouteParams() = default;

    size_t size() const noexcept {
      return paramv.size();
    }

    size_t named_size() const noexcept {
      return paramm.size();
    }

    size_t unnamed_size() const noexcept {
      return size() - named_size();
    }

    param_vec_type::const_iterator begin() const noexcept {
      return paramv.cbegin();
    }

    param_vec_type::const_iterator end() const noexcept {
      return paramv.cend();
    }

    param_map_type::const_iterator named_begin() const noexcept {
      return paramm.cbegin();
    }

    param_map_type::const_iterator named_end() const noexcept {
      return paramm.cend();
    }

    param_vec_type::const_iterator unnamed_begin() const noexcept {
      if (paramv.size() > paramm.size()) {
        return paramv.cbegin() + paramm.size();
      }
      return paramv.cend();
    }

    param_vec_type::const_iterator unnamed_end() const noexcept {
      return end();
    }

    std::string_view operator[](size_t index) const noexcept {
      return paramv[index];
    }

    std::string_view operator[](std::string_view param) const noexcept {
      if (auto it = paramm.find(param); it != paramm.end()) {
        return it->second;
      }
      return {};
    }
  };

private:
  using string_id_t = unsigned;
  static constexpr string_id_t BadString = string_id_t(-1);
  static constexpr string_id_t EmptyString = string_id_t(0);
  static constexpr string_id_t ParamString = string_id_t(1);

protected:
  using route_id_t = unsigned;
  static constexpr route_id_t BadRoute = route_id_t(-1);

  struct RouteBase {
  private:
    friend class RouterBase;
    static constexpr size_t InlineParams = 2;
    boost::container::small_vector<string_id_t, InlineParams> params;

  public:
    RouteBase() = default;
  };

private:
  using state_id_t = unsigned;
  static constexpr state_id_t BadState = state_id_t(-1);

  struct State {
    state_id_t id;
    route_id_t route = BadRoute;
  };

  // Persistent route segments and parameter names.
  std::vector<std::unique_ptr<char[]>> _idToString;
  // Map from string to index in persistent store.
  std::unordered_map<std::string_view, string_id_t> _stringToId;
  state_id_t _nextStateId = 1;
  State _initial{0, BadRoute};
  // Map from [current state, current string] -> next state.
  // Parameters use ParamString (":") and are named only on route completion.
  std::unordered_map<
    std::pair<state_id_t, string_id_t>, State,
    boost::hash<std::pair<state_id_t, string_id_t>>
  > _transitions;

protected:
  explicit RouterBase();

  RouterBase(const RouterBase &) = delete;
  RouterBase &operator=(const RouterBase &) = delete;

  void fillParamMap(RouteParams &params, const RouteBase &route) const;

  bool insert(std::string_view path, route_id_t routeId, RouteBase &route);
  route_id_t route(std::string_view path, RouteParams &params) const;

private:
  route_id_t routeStep(std::string_view path, const State &state,
                       RouteParams &params) const;

  string_id_t getStringId(std::string_view str) const;
  string_id_t putString(std::string_view str);

  const State *getTransition(state_id_t stateId, string_id_t strId) const;
  State *getOrPutTransition(state_id_t stateId, string_id_t strId);
  bool putSelfTransition(State *state, route_id_t routeId);
};

template<class Req>
struct RouterTraits;

template<class Req, class Traits = RouterTraits<Req>>
class Router : public RouterBase {
public:
  using callback_type = typename Traits::callback_type;

private:
  using RouterBase::route_id_t;
  using RouterBase::BadRoute;

  struct Route : RouterBase::RouteBase {
    callback_type callback;
  };

  std::vector<Route> _routes;

public:
  callback_type *add(std::string_view path) {
    auto rid = route_id_t(_routes.size());
    Route route;
    if (!this->insert(path, rid, route)) {
      return nullptr;
    }
    return &_routes.emplace_back(std::move(route)).callback;
  }

  callback_type &operator[](std::string_view path) {
    return *add(path);
  }

  template<class... Args>
  bool operator()(Args &&...args) const {
    decltype(auto) req = Traits::get_request(args...);
    std::string_view path = Traits::get_route_path(req);
    RouteParams &params = Traits::get_route_params(req);
    if (auto rid = this->route(path, params); rid != BadRoute) {
      this->fillParamMap(params, _routes[rid]);
      Traits::call(_routes[rid].callback, req, std::forward<Args>(args)...);
      return true;
    }
    return false;
  }
};

} // namespace a8

#endif // A8_UTIL_ROUTER_H_