#pragma once

namespace a8::flp {

constexpr int FLMaxParam = 0x40000000;
constexpr float FLMaxParamInv = 1.f / (float)FLMaxParam;

template<class V>
struct ParamConverter;

template<>
struct ParamConverter<float> {
  float _min, _max;
  constexpr explicit ParamConverter(float min, float max) noexcept
    : _min{min}, _max{max}
  {}

  // From FL values
  constexpr float get(int value) const noexcept {
    if (value == 0) return _min;
    if (value == FLMaxParam) return _max;
    return ((float)value * FLMaxParamInv * (_max - _min)) + _min;
  }

  // To FL values
  constexpr int set(float value) const noexcept {
    if (value <= _min) return 0;
    if (value >= _max) return FLMaxParam;
    return (int)((value - _min) * ((float)FLMaxParam) / (_max - _min));
  }
};

template<>
struct ParamConverter<int> {
  int _min, _max;
  constexpr explicit ParamConverter(int min, int max) noexcept
    : _min{min}, _max{max}
  {}

  // From FL values
  constexpr int get(int value) const noexcept {
    // Input range:  [0, FromMIDI_Max]
    // Output range: [_min, _max]
    const long long rangeInp = FLMaxParam + 1LL;
    const long long rangeOut = _max - _min + 1LL;
    return (int)(((long long)value) * rangeOut / rangeInp) + _min;
  }

  // To FL values
  constexpr int set(int value) const noexcept {
    const long long rangeInp = _max - _min;
    const long long rangeOut = FLMaxParam;
    return (int)(((long long)(value - _min)) * rangeOut / rangeInp);
  }
};

} // namespace a8::flp
