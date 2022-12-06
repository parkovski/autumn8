#pragma once

#include "a8/util/router.h"
#include "a8/webview/webview.h"

namespace a8::app::browser {

class Browser;

struct Routes {
  using Request = wv::WebViewRouterRequest;
  using Response = wv::WebViewResponse;

  explicit Routes(Router<Request> &r, Browser &browser);

  void viewFile(const Request &req, Response &res);
  void cwd(const Request &req, Response &res);
  void ls(const Request &req, Response &res);
  void go(const Request &req, Response &res);
  void devEnabled(const Request &req, Response &res);
  void devVisible(const Request &req, Response &res);
  void devShow(const Request &req, Response &res);
  void log(const Request &req, Response &res);

private:
  Browser &browser;
};

} // namespace a8::app::browser
