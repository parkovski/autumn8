#include "a8/a8.h"
#include "a8/gfx/window.h"
#include "a8/gfx/view.h"
#include "a8/gfx/renderer.h"

#include <filesystem>
#include <string>
#include <sstream>

namespace fs = std::filesystem;

using namespace a8::gfx;

const char *const BaseDir = R"(C:\Users\parker\Documents\Image-Line\0Data\AKWF-FREE-master)";

int a8main(int argc, char *argv[]) {
  auto mainWindow = Window::create("MiniGfx", {0, 0}, {500, 500});
  if (!mainWindow) {
    return 1;
  }

  std::string title{"Path: "};
  title.append(BaseDir);

  fs::path path{BaseDir};
  unsigned items = 0;

  fs::directory_iterator dirent{path};
  fs::directory_iterator end;
  while (dirent != end) {
    ++dirent;
    ++items;
  }

  std::stringstream ss;
  ss << items;
  ss << " items";
  std::string itemCount = ss.str();
  mainWindow->setView(new CanvasView([&](View &, Renderer &r) {
#if 0
    r.strokeColor(Renderer::rgb(0xff, 0xff, 0xff));
    r.fillColor(Renderer::rgb(0xff, 0x80, 0x20));
    r.beginPath();
    r.circle(250, 250, 100);
    r.stroke();
    r.fill();
#endif
    r.text(10, 25, title);
    r.text(10, 50, itemCount);
  }));

  return runMessageLoop(*mainWindow);
}
