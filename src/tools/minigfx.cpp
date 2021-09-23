#include "a8/a8.h"
#include "a8/gfx/msgloop.h"
#include "a8/gfx/window.h"
#include "a8/gfx/views/webview.h"

#include <uwebsockets/App.h>
//#include <uwebsockets/WebSocket.h>

#include <thread>

#pragma comment(lib, "Psapi.lib")

using namespace a8::gfx;

const char *const document = R"(
<!doctype html>
<html><head><title>foobar</title></head>
<body>well well jimmy look at this.
<h1>it's a dang html document</h1></body>
</html>
)";

static void server(int port, uWS::Loop **loop, us_listen_socket_t **socket) {
  uWS::App()
  .get("/", [=](auto *res, auto *req) {
    res->end(document);
  })
  .listen(port, [=](auto *listenSocket) {
    *loop = uWS::Loop::get();
    *socket = listenSocket;
  })
  .run();
}

int a8main(int argc, char *argv[]) {
  Window *mainWindow = new Window;
  mainWindow->resize(0, 0, 500, 500);
  mainWindow->quitOnClose();

  auto *webview = new WebView("http://localhost:9001/");
  mainWindow->setView(webview);

  if (!mainWindow->open("MiniGfx")) {
    return 1;
  }

  uWS::Loop *loop;
  us_listen_socket_t *socket;
  std::thread serverThread{server, 9001, &loop, &socket};

  //mainWindow->runOnMessageThread([=](){webview->navigate("http://localhost:9001/");});

  auto x = runMessageLoop();
  a8::Log::debug("message loop returned {}", x);
  loop->defer([socket](){ us_listen_socket_close(0, socket); });
  serverThread.join();
  a8::Log::debug("server thread finished");
  return x;
}
