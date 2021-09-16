#pragma once

#include "fp_cplug.h"

#include "a8/gfx/window.h"

namespace a8::flp {

struct TVoice {
  intptr_t HostTag = 0;
  TVoiceParams *Params = nullptr;
  bool IsReleased = false;
  unsigned Position = 0;
  float CurrentPitch = 0;
  int Speed = 0;
  float LastLVol = 0;
  float LastRVol = 0;

  void clear() noexcept {
    HostTag = 0;
    Params = nullptr;
    IsReleased = false;
    Position = 0;
    CurrentPitch = 0;
    Speed = 0;
    LastLVol = 0;
    LastRVol = 0;
  }

  explicit operator bool() noexcept {
    // We hope that at least one of these has to be not zero.
    auto stuff = (size_t)HostTag | (size_t)Params | (size_t)Position | (size_t) Speed;
    return stuff != 0;
  }
};

class FLPlugin : public TCPPFruityPlug {
public:
  static constexpr int MaxVoices = 16;

  struct Param { enum {
    volume,
    count,
  }; };

private:
  TVoice _voices[MaxVoices];

  void clearVoices() noexcept {
    for (auto &&voice : _voices) {
      voice.clear();
    }
  }

  int nextVoice() noexcept {
    for (int i = 0; i < MaxVoices; ++i) {
      if (!_voices[i]) {
        return i;
      }
    }
    return -1;
  }

  int _params[Param::count];

  a8::gfx::Window *_window;

public:
  FLPlugin(TFruityPlugHost *host, int tag);
  ~FLPlugin();

  intptr_t __stdcall Dispatcher(intptr_t id, intptr_t index, intptr_t value) override;
  void __stdcall SaveRestoreState(IStream *stream, BOOL save) override;
  void __stdcall GetName(int section, int index, int value, char *name) override;
  int __stdcall ProcessEvent(int eventId, int value, int flags) override;
  int __stdcall ProcessParam(int index, int value, int recflags) override;

  void __stdcall Gen_Render(PWAV32FS buffer, int &length) override;
  //void __stdcall Eff_Render(PWAV32FS src, PWAV32FS dst, int length) override;

  TVoiceHandle __stdcall TriggerVoice(PVoiceParams params, intptr_t tag) override;
  void __stdcall Voice_Release(TVoiceHandle handle) override;
  void __stdcall Voice_Kill(TVoiceHandle handle) override;

  int __stdcall Voice_ProcessEvent(TVoiceHandle handle, int eventId, int value, int flags);
};

} // namespace a8::flp
