#include "a8/a8.h"

#include <ni/media/audio/wav/wav_ofstream.h>

#include <fmt/ostream.h>

#include <vector>
#include <fstream>
#include <optional>

struct samples {
  float a;
  float c;
  float g;
  float u;
};

struct virdata {
  explicit virdata(std::ifstream input)
    : input{std::move(input)}
  {}

  std::ifstream input;
  std::vector<float> as;
  std::vector<float> cs;
  std::vector<float> gs;
  std::vector<float> us;

  template<typename FA, typename FC, typename FG, typename FU, typename FS>
  void each(FA &&fa, FC &&fc, FG &&fg, FU &&fu, FS &&fs) {
    while (!input.eof()) {
      char x = input.get();
      switch (x) {
        case 'a': fa(); break;
        case 'c': fc(); break;
        case 'g': fg(); break;
        case 't': [[fallthrough]];
        case 'u': fu(); break;
        default: continue;
      }
      auto const &samples = fs();
      as.push_back(samples.a);
      cs.push_back(samples.c);
      gs.push_back(samples.g);
      us.push_back(samples.u);
    }
  }

  template<typename F>
  void each(F &&f) {
    char c;
    each([&]() { c = 'a'; },
         nop, nop, nop, std::forward<F>(f));
  }

private:
  static void nop() {}
};

void countFrequency(virdata &vd) {
  double total = 0;
  double a = 0, c = 0, g = 0, u = 0;
  vd.each(
    [&]() { ++a; },
    [&]() { ++c; },
    [&]() { ++g; },
    [&]() { ++u; },
    [&]() -> samples {
      ++total;
      return {
        (float)(a / total - .5),
        (float)(c / total - .5),
        (float)(g / total - .5),
        (float)(u / total - .5),
      };
    }
  );
}

void countBinary(virdata &vd) {
  float a = -.5f;
  float c = -.5f;
  float g = -.5f;
  float u = -.5f;
  vd.each(
    [&]() { a = .5f; },
    [&]() { c = .5f; },
    [&]() { g = .5f; },
    [&]() { u = .5f; },
    [&]() -> samples {
      samples s{a, c, g, u};
      a = c = g = u = -.5f;
      return s;
    }
  );
}

void (*getop(std::string_view name))(virdata &) {
  if (name == "freq") {
    return &countFrequency;
  } else if (name == "bin") {
    return &countBinary;
  }

  return nullptr;
}

void printops() {
  fmt::print(stderr, "op: [freq, bin]\n");
}

int a8main(int argc, char *argv[]) {
  if (argc != 3) {
    fmt::print(stderr, "Usage: rna_to_wav2 <genome> <op>\n");
    printops();
    return 1;
  }
  auto op = getop(argv[2]);
  if (!op) {
    printops();
    return 1;
  }

  virdata vd{std::ifstream{argv[1]}};
  if (!vd.input.good()) {
    return 1;
  }

  audio::wav_ofstream_info info;
  audio::wav_ofstream a_out{"a.wav", info};
  audio::wav_ofstream u_out{"u.wav", info};
  audio::wav_ofstream c_out{"c.wav", info};
  audio::wav_ofstream g_out{"g.wav", info};

  if (!a_out.good()) return 2;
  if (!u_out.good()) return 3;
  if (!c_out.good()) return 4;
  if (!g_out.good()) return 5;

  op(vd);

  a_out << vd.as;
  u_out << vd.us;
  c_out << vd.cs;
  g_out << vd.gs;

  return 0;
}

