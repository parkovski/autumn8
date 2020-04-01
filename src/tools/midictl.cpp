#include "a8/a8.h"
#include "a8/midi/midi.h"

using namespace a8;
using namespace a8::midi;

int a8main(int argc, char *argv[]) {
  Backend backend;
  if (argc == 2) {
    std::string_view backend_arg{argv[1]};
    if (backend_arg == "winmm") {
      backend = Backend::Winmm;
    } else if (backend_arg == "uwp") {
      backend = Backend::UWP;
    } else {
      Log::error("Invalid backend {}", backend_arg);
      return 1;
    }
  } else {
    backend = Backend::Default;
  }

  auto inMan = PortManager<InputPort>::create(backend);
  auto outMan = PortManager<OutputPort>::create(backend);

  auto inf = inMan->scan();
  auto outf = outMan->scan();

  inf.wait();
  outf.wait();

  return 0;
}

