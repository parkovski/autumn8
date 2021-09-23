#include "a8/flp/param.h"
#include <catch2/catch_all.hpp>

using namespace a8::flp;

/*
 * Some possible conversion scales:
 * Take for example ranges [0, 3] and [0, 4].
 *
 * Even nr. steps, odd nr. divisions:
 *  0    1     2    3
 *  - | - - | - - | -
 * [ )[  , )[  , )[  ]
 * 0 0.5 1 1.5 2 2.5 3
 *          ^
 *          Midpoint (1.5 => 2)
 *
 * Odd nr. steps, even nr. divisions:
 *  0    1     2     3    4
 *  - | - - | - - | - - | -
 * [ )[  , )[  , )[  , )[  ]
 * 0 0.5 1 1.5 2 2.5 3 3.5 4
 *             ^
 *             Midpoint (2 => 2)
 *
 * In this scale, the upper and lower most values have only half their target
 * ranges. The space is divided into equal size ranges, but one of those is
 * further divided by 2.
 *
 * Even nr. steps, even nr. divisions, special last tick:
 *   0   1   2   3
 * | - | - | - | - -
 * [  )[  )[  )[   ]
 * 0   1   2   3   4
 *        ^
 *        Midpoint (right under 2 => 1)
 *
 * Odd nr. steps, odd nr. divisions, special last tick:
 *   0   1   2   3   4
 * | - | - | - | - | - -
 * [  )[  )[  )[  )[   ]
 * 0   1   2   3   4   5
 *           ^
 *           Midpoint (right under 2.5 => 2)
 *
 * In this scale, every range is equal except the last which is special and
 * gets an extra tick that also rounds down.
 *
 * Even nr. steps, odd nr. divisions, special last tick:
 * | - | - | - |
 * [  )[  )[  )]
 * 0   1   2   3
 *       ^
 *       Midpoint (1.5 => 1)
 *
 * Odd nr. steps, even nr. divisions, special last tick:
 * | - | - | - | - |
 * [  )[  )[  )[  )]
 * 0   1   2   3   4
 *         ^
 *         Midpoint (2 => 2)
 *
 * In this scale, only the very last tick has the upper bound value. All other
 * ranges are equal.
 */

static const int FromMIDI_Max = 0x40000000;
static const int FromMIDI_Half = 0x20000000;

TEST_CASE("Int param conversion", "[param]") {
  using PCI = ParamConverter<int>;

  PCI pci{0, 10};
  REQUIRE(pci.get(0) == 0);
  REQUIRE(pci.get(FromMIDI_Max) == 10);
  REQUIRE(pci.get(FromMIDI_Half) == 5);

  REQUIRE(pci.set(0) == 0);
  REQUIRE(pci.set(10) == FromMIDI_Max);
  REQUIRE(pci.set(5) == FromMIDI_Half);

  pci = PCI{FromMIDI_Half, FromMIDI_Max};
  REQUIRE(pci.get(0) == FromMIDI_Half);
  REQUIRE(pci.get(FromMIDI_Max) == FromMIDI_Max);
  REQUIRE(pci.get(4) == FromMIDI_Half + 2);
  REQUIRE(pci.get(FromMIDI_Max - 10) == FromMIDI_Max - 5);

  REQUIRE(pci.set(FromMIDI_Half) == 0);
  REQUIRE(pci.get(FromMIDI_Max) == FromMIDI_Max);
  REQUIRE(pci.set(FromMIDI_Half + 2) == 4);
  REQUIRE(pci.set(FromMIDI_Max - 5) == FromMIDI_Max - 10);
}

TEST_CASE("Float param conversion", "[param]") {
  using PCF = ParamConverter<float>;

  PCF pcf{0.f, 1.f};
  REQUIRE(pcf.get(0) == 0.f);
  REQUIRE(pcf.get(FromMIDI_Max) == 1.f);
  REQUIRE(pcf.get(FromMIDI_Half) == 0.5f);

  REQUIRE(pcf.set(0.f) == 0);
  REQUIRE(pcf.set(1.f) == FromMIDI_Max);
  REQUIRE(pcf.set(0.5f) == FromMIDI_Half);

  pcf = PCF{-.5f, .5f};
  REQUIRE(pcf.get(0) == -.5f);
  REQUIRE(pcf.get(FromMIDI_Max / 4) == -.25f);
  REQUIRE(pcf.get(FromMIDI_Max / 2) == 0.f);
  REQUIRE(pcf.get(FromMIDI_Max / 4 * 3) == .25f);
  REQUIRE(pcf.get(FromMIDI_Max) == .5f);

  REQUIRE(pcf.set(-.5f) == 0);
  REQUIRE(pcf.set(-.25f) == (FromMIDI_Max / 4));
  REQUIRE(pcf.set(0.f) == (FromMIDI_Max / 2));
  REQUIRE(pcf.set(.25f) == (FromMIDI_Max / 4 * 3));
  REQUIRE(pcf.set(.5f) == FromMIDI_Max);

  pcf = PCF{-1.f, 1.f};
  REQUIRE(pcf.get(0) == -1.f);
  REQUIRE(pcf.get(FromMIDI_Max / 8 * 7) == .75f);

  REQUIRE(pcf.set(-1.f) == 0);
  REQUIRE(pcf.set(.75f) == (FromMIDI_Max / 8 * 7));
}
