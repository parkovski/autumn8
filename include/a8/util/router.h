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

class Router {
public:
  using param_map_type =
    std::unordered_map<std::string_view, std::string_view>;
  using callback_type = void(const param_map_type &);

private:
  using state_id_t = unsigned;
  static constexpr state_id_t BadState = state_id_t(-1);

  using route_id_t = unsigned;
  static constexpr route_id_t BadRoute = route_id_t(-1);

  using string_id_t = unsigned;
  static constexpr string_id_t BadString = string_id_t(-1);
  static constexpr string_id_t EmptyString = string_id_t(0);
  static constexpr string_id_t ParamString = string_id_t(1);

  struct Route {
    static constexpr size_t InlineParams = 2;

    std::function<callback_type> callback;
    boost::container::small_vector<
      std::pair<state_id_t, string_id_t>, InlineParams> params;
  };

  struct State {
    state_id_t id;
    route_id_t route = BadRoute;
  };

  struct ParamList {
    ParamList *next;
    state_id_t stateId;
    std::string_view value;
  };

  std::vector<std::unique_ptr<char[]>> _idToString;
  std::unordered_map<std::string_view, string_id_t> _stringToId;
  std::vector<Route> _routes;
  state_id_t _nextStateId = 1;
  State _initial{0, BadRoute};
  std::unordered_map<
    std::pair<state_id_t, string_id_t>, State,
    boost::hash<std::pair<state_id_t, string_id_t>>
  > _transitions;

public:
  explicit Router();

  std::function<callback_type> &operator[](std::string_view path);
  bool operator()(std::string_view path) const;

private:
  string_id_t getStringId(std::string_view str) const;
  string_id_t putString(std::string_view str);

  const State *getTransition(state_id_t stateId, string_id_t strId) const;
  State *getOrPutTransition(state_id_t stateId, string_id_t strId);

  bool route(std::string_view path, const State &state, ParamList *plist,
             ParamList *phead) const;

  bool callRoute(route_id_t routeId, ParamList *phead) const;
};

} // namespace a8

#endif // A8_UTIL_ROUTER_H_