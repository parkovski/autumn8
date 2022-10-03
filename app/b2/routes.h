#pragma once

#include "a8/util/router.h"
#include "a8/webview/webview.h"

namespace a8::tools::browser {

class Browser;

struct Routes {
  using Request = webview::WebViewRouterRequest;
  using Response = webview::WebViewResponse;

  explicit Routes(Router<Request> &r, Browser &browser);

  void viewFile(const Request &req, Response &res);
  void pwd(const Request &req, Response &res);
  void goBack(const Request &req, Response &res);
  void goForward(const Request &req, Response &res);
  void goUp(const Request &req, Response &res);
  void goTo(const Request &req, Response &res);
  void goIn(const Request &req, Response &res);
  void ls(const Request &req, Response &res);
  void historyBack(const Request &req, Response &res);
  void historyForward(const Request &req, Response &res);
  void navstate(const Request &req, Response &res);
  void devEnabled(const Request &req, Response &res);
  void devVisible(const Request &req, Response &res);
  void devShow(const Request &req, Response &res);

private:
  Browser &browser;
};

} // namespace a8::tools::browser
