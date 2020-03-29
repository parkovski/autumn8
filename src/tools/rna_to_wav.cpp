#include "a8/a8.h"

#include <ni/media/audio/wav/wav_ofstream.h>

#include <fmt/ostream.h>

#include <vector>
#include <fstream>
#include <optional>

float samplevalues[] = {
  -1.f, -.333333f, .333333f, 1.f,
  0.f, .333333f, .666667f, 1.f,
  0.f, .666667f, 1.333333f, 2.f,
  -.5f, -.166667f, .166667f, .5f,
};
const char *order = "augc";
float *samplebase = samplevalues;
constexpr char maxsamplebase = 3;

std::optional<float> gtos(char g) {
  if (g == 't') g = 'u';
  for (unsigned i = 0; i < 4; ++i) {
    if (g == order[i]) return samplebase[i];
  }
  return std::nullopt;
}

bool setorder(const char *o) {
  constexpr unsigned a_flag = 1;
  constexpr unsigned u_flag = 2;
  constexpr unsigned g_flag = 4;
  constexpr unsigned c_flag = 8;
  unsigned flags = 0;
  unsigned len = 0;
  while (o[len]) {
    if (len == 4 && o[len] != 0) {
      return false;
    }
    switch (o[len]) {
      case 'a':
        if (flags & a_flag) return false;
        flags |= a_flag;
        break;
      case 'u':
        if (flags & u_flag) return false;
        flags |= u_flag;
        break;
      case 'g':
        if (flags & g_flag) return false;
        flags |= g_flag;
        break;
      case 'c':
        if (flags & c_flag) return false;
        flags |= c_flag;
        break;
    }
    ++len;
  }
  if (flags == 15) {
    order = o;
    return true;
  }
  return false;
}

bool setsamplebase(char *arg) {
  if (arg[0] < '0' || arg[0] > ('0' + maxsamplebase) || arg[1]) {
    return false;
  }
  samplebase = &samplevalues[4 * (arg[0] - '0')];
  return true;
}

int a8main(int argc, char *argv[]) {
  if (argc < 3 || argc > 5) {
    fmt::print(stderr, "Usage: rna_to_wav <input> <output> [order] [samplemode]\n");
    return 1;
  }

  if (argc == 4) {
    if (!setsamplebase(argv[3]) && !setorder(argv[3])) {
      fmt::print(stderr, "Order must contain one each a, u, g, c\n");
      return 1;
    }
  } else if (argc == 5) {
    if (!setorder(argv[3])) {
      fmt::print(stderr, "Order must contain one each a, u, g, c\n");
      return 1;
    }
    if (!setsamplebase(argv[4])) {
      fmt::print(stderr, "Valid sample modes are from 0-{}\n", (int)maxsamplebase);
      return 1;
    }
  }

  std::ifstream input{argv[1]};
  if (!input.good()) {
    fmt::print(stderr, "Couldn't open input file\n");
    return 1;
  }

  audio::wav_ofstream_info info;
  audio::wav_ofstream output{argv[2], info};
  if (!output.good()) {
    fmt::print(stderr, "Couldn't open output file\n");
    return 1;
  }

  std::vector<float> samples;
  while (!input.eof()) {
    if (auto s = gtos((char)input.get()); s.has_value()) {
      samples.push_back(s.value());
    }
  }

  output << samples;

  return 0;
}
