#include "a8/a8.h"
#include "a8/window/window.h"
#include "a8/window/msgloop.h"
#include "a8/webview/webview.h"

A8_PRAGMA_MS(warning(push))
A8_PRAGMA_MS(warning(disable: 4244))
#include <uWebSockets/App.h>
#include <uWebSockets/WebSocket.h>
A8_PRAGMA_MS(warning(pop))

#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstring>
#include <sstream>

A8_PRAGMA_MS(comment(lib, "Psapi.lib"))

using namespace a8::window;
using namespace a8::webview;

const char *const docHead = R"(
<!doctype html>
<html>
<head>
<title>foobar</title>
<style>
.error { color: red; }
.waiting { color: orange; }
.ok { color: green; }
</style>
</head>
<body>well well jimmy look at this.
<h1>it's a dang html document</h1>
<button id="b">Do Websocket Things</button><br>
<span id="s">Click it.</span>
<script>
document.addEventListener('DOMContentLoaded', function() {
)";

const char *const docFoot = R"(
  const b = document.getElementById('b');
  const s = document.getElementById('s');
  let curclass = null;

  function setclass(c) {
    if (curclass) {
      s.classList.remove(curclass);
    }
    curclass = c;
    if (c) {
      s.classList.add(c);
    }
  }
  b.onclick = function() {
    const ws = new WebSocket(wsUrl);
    setTimeout(function() { ws.send('hello'); }, 1000);
    setTimeout(function() { ws.close(1000, 'mang i aint got time fo dis'); }, 3000);

    ws.onerror = function(e) { s.innerText = e.type; setclass('error'); };
    ws.onmessage = function(e) { s.innerText = e.type + ': ' + e.data.toString(); setclass('ok'); };
    ws.onopen = function(e) { s.innerText = 'waiting'; setclass('waiting'); };
    ws.onclose = function(e) { s.innerText = 'closed'; setclass(null); };
  };
});
</script>
</body></html>
)";

static void configureHttpServer(uWS::App &app, int wsPort) {
  A8_DEBUG("configureHttpServer wsPort={}.", wsPort);
  std::stringstream docss;
  docss << docHead;
  docss << "    const wsUrl = 'ws://localhost:" << wsPort << "/ws';\n";
  docss << docFoot;
  app.get("/", [doc = docss.str()](auto *res, auto *req) {
    A8_DEBUG("HTTP request. Doc size = {}.", doc.length());
    res->end(doc);
  });
}

static void configureWebSocketServer(uWS::App &app) {
  A8_DEBUG("configureWebSocketServer.");
  uWS::App::WebSocketBehavior<int> behavior;
  behavior.open = [](auto *ws) {
    A8_DEBUG("WebSocket open");
  };
  behavior.message = [](auto *ws, std::string_view msg, uWS::OpCode op) {
    const char *opName;
    switch (op) {
      case uWS::OpCode::TEXT:
        opName = "TEXT";
        ws->send("hello yourself", uWS::OpCode::TEXT);
        break;
      case uWS::OpCode::BINARY: opName = "BINARY"; break;
      case uWS::OpCode::CLOSE: opName = "CLOSE"; break;
      case uWS::OpCode::PING: opName = "PING"; break;
      case uWS::OpCode::PONG: opName = "PONG"; break;
      default: opName = "unknown"; break;
    }
    A8_DEBUG("WebSocket message ({}): {}", opName, msg);
  };
  behavior.close = [](auto *ws, int code, std::string_view reason) {
    A8_DEBUG("WebSocket close ({}){}{}", code,
             reason.length() ? ": " : "", reason);
  };

  app.ws("/ws", std::move(behavior));
}

static void configureListener(uWS::App &app, int port, uWS::Loop **loop,
                              us_listen_socket_t **listenSocket,
                              std::mutex &mutex,
                              std::condition_variable &cond) {
  A8_DEBUG("configureListener port={}.", port);
  *loop = nullptr;
  app.listen(port, [=, &mutex, &cond](auto *socket) {
    if ((*listenSocket = socket)) {
      *loop = uWS::Loop::get();
    }
    std::lock_guard<std::mutex> lock(mutex);
    cond.notify_one();
  });
}

// Returns when the thread is initialized and loop and socket have been
// assigned. If they are null, something went wrong.
// If devPort is not 0, no HTTP server will be configured.
static std::thread server(int wsPort, int devPort, uWS::Loop **loop,
                          us_listen_socket_t **socket) {
  std::mutex mutex;
  std::condition_variable cond;
  std::unique_lock<std::mutex> lock(mutex);
  auto thread = std::thread{[=, &mutex, &cond](){
    uWS::App app;
    if (!devPort) {
      configureHttpServer(app, wsPort);
    } else {
      A8_DEBUG("Using external dev server on port {}.", devPort);
    }
    configureWebSocketServer(app);
    configureListener(app, wsPort, loop, socket, mutex, cond);
    A8_DEBUG("Starting uWS app.");
    app.run();
  }};
  cond.wait(lock);
  A8_DEBUG("uWS app initialization finished.");
  return thread;
}

class WebViewRouter {
  struct RouterData {
    const WebViewRequest *req;
    WebViewResponse *res;
  };
  uWS::HttpRouter<RouterData> router;

  void get(std::string pattern,
           uWS::MoveOnlyFunction<bool(const WebViewRequest &,
                                      WebViewResponse &)> fn) {
    router.add({"get"}, std::move(pattern),
               [fn = std::move(fn)](auto *router) mutable -> bool {
      RouterData &data = router->getUserData();
      return fn(*data.req, *data.res);
    });
  }

  bool operator()(WebView *, const WebViewRequest &req, WebViewResponse &res) {
    std::string_view uri{req.uri().path().str()};
    for (auto const &method : router.methods) {
      if (uri.length() > method.length() + 2 &&
          uri[method.length()] == '/' &&
          uri.substr(1, method.length()) == method) {
        router.getUserData() = RouterData { &req, &res };
        return router.route(method, uri.substr(method.length() + 1));
      }
    }
    return false;
  }
};

int a8main(int argc, char *argv[]) {
  int port = 0;
  int devPort = 0;
  for (int i = 1; i < argc; ++i) {
#ifndef NDEBUG
    constexpr size_t devPortArgLen = 11;
    if (!strncmp(argv[i], "--dev-port=", devPortArgLen)) {
      char *pport = argv[i] + devPortArgLen;
      char *endp;
      devPort = (int)strtol(pport, &endp, 10);
      if (devPort <= 0 || *endp != 0) {
        A8_ERROR("Invalid port: '{}'.", pport);
        return 1;
      }
      continue;
    }
#endif

    if (!strncmp(argv[i], "-p", 2)) {
      char *pport = argv[i] + 2;
      char *endp;
      port = (int)strtol(pport, &endp, 10);
      if (port <= 0 || *endp != 0) {
        A8_ERROR("Invalid port: '{}'.", pport);
        return 1;
      }
    }
  }

  if (!port) {
    port = 9001;
    if (devPort == 9001) {
      port = 9002;
    }
  }
  std::string urlBase{
    fmt::format("http://localhost:{}/", devPort ? devPort : port)
  };

  uWS::Loop *loop;
  us_listen_socket_t *socket;
  auto serverThread = server(port, devPort, &loop, &socket);
  if (!socket) {
    A8_ERROR("Server creation failed.");
    return 1;
  }
  A8_INFO("Listening on port {}.", port);

  Window *mainWindow = new Window("UWS Server");
  mainWindow->resize(0, 0, 500, 500);
  mainWindow->quitOnClose();

  auto *webview = new WebView(mainWindow);
  webview->navigate(urlBase);

  mainWindow->show();

  auto x = runMessageLoop();
  A8_DEBUG("Message loop returned {}.", x);
  loop->defer([socket](){ us_listen_socket_close(0, socket); });
  serverThread.join();
  A8_DEBUG("Server thread finished.");
  return x;
}
