#include "a8/a8.h"

#include <bimg/bimg.h>
#include <bx/file.h>
#include <ni/media/audio/ifstream.h>

#include <fmt/ostream.h>

#include <string_view>
#include <filesystem>
#include <cstdlib>

std::ostream &operator <<(std::ostream &os, const bx::Error &error) {
  return os << error.getMessage().getPtr();
}

bool to_ulong(const char *str, unsigned long &n) {
  char *endp;
  unsigned long nn = strtoul(str, &endp, 10);
  if (endp == str) {
    return false;
  }
  n = nn;
  return true;
}

struct Options {
  bool parse(int argc, const char *const *argv) {
    int i = 1;
    for (; i < argc; ++i) {
      std::string_view arg{argv[i]};
      if (arg == "-w") {
        if (++i == argc || !to_ulong(argv[i], width)) {
          return false;
        }
      } else if (arg == "-h") {
        if (++i == argc || !to_ulong(argv[i], height)) {
          return false;
        }
      } else {
        break;
      }
    }

    if (argc - i != 2) {
      return false;
    }

    input = argv[i];
    output = argv[i + 1];

    return true;
  }

  unsigned long width = 250;
  unsigned long height = 100;
  std::filesystem::path input;
  std::filesystem::path output;
};

void fill(uint32_t *data, unsigned long width, unsigned long height, unsigned linewidth) {
  for (unsigned long y = 0; y < height; ++y) {
    for (unsigned long x = 0; x < width; ++x) {
      data[x * height + y] = 0xffffaf00;
    }
  }

  //for (unsigned lw = 0; lw < linewidth; ++lw) {
    for (unsigned long x = 0; x < width; ++x) {
      auto y = x * height / width;
      data[x * height + y] = 0xff000000;
    }
  //}*/

  for (unsigned long y = 0; y < height; ++y) {
    data[y + height/2] = 0xffff00ff;
  }
}

int a8main(int argc, char *argv[]) {
  Options opts;
  if (!opts.parse(argc, argv)) {
    fmt::print(stderr, "Usage: auconvert [-w width] [-h height] <input> <output>\n");
    return 1;
  }

  if (opts.width > 0x1000 || opts.height > 0x1000) {
    fmt::print(stderr, "Too big\n");
    return 1;
  }
  auto data = new (std::nothrow) uint32_t[opts.width * opts.height];
  if (!data) {
    fmt::print(stderr, "Not enough memory\n");
    return 1;
  }

  fill(data, opts.width, opts.height, 3);

  bx::FileWriter writer;
  bx::Error error;
  if (!writer.open(opts.output.u8string().c_str(), false, &error)) {
    fmt::print(stderr, "Failed opening file: {}\n", error);
    return 1;
  }
  bimg::imageWritePng(&writer, opts.width, opts.height, 0, data,
                      bimg::TextureFormat::RGBA8, false, &error);
  if (!error.isOk()) {
    fmt::print(stderr, "Writing PNG failed: {}\n", error);
    return 1;
  }
  return 0;
}
