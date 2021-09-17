#include "a8/a8.h"
#include "a8/flp/flplug.h"

#include "a8/gfx/views/webview.h"

using namespace a8::flp;
using a8::Log;

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

class FLSaw : public FLPlugin {
public:
  static constexpr int Version = 1;
  static constexpr const char *PlugName = "A8Saw";
  static constexpr int Flags = FPF_Type_FullGen;
  static constexpr int DefaultPolyVoices = 16;
  static constexpr int OutControlCount = 0;
  static constexpr int OutVoiceCount = 0;

  struct ParamId { enum {
    Volume,
    WaveTable,
    END,
  }; };
  static constexpr int DefaultParamCount = ParamId::END;

  static constexpr int WaveTableCount = 6;

private:
  int _params[DefaultParamCount] = { 80, 3 };
  TVoice _voices[DefaultPolyVoices];

  void clearVoices() noexcept {
    for (auto &&voice : _voices) {
      voice.clear();
    }
  }

  int nextVoice() noexcept {
    for (int i = 0; i < DefaultPolyVoices; ++i) {
      if (!_voices[i]) {
        return i;
      }
    }
    return -1;
  }

public:
  a8::gfx::Window *createEditor() override {
    auto win = new a8::gfx::Window();
    win->resize(0, 0, 400, 400);
    win->setView(new a8::gfx::WebView);
    return win;
  }

  void saveState(IStream *stream) override {
    stream->Write(_params, sizeof(_params), nullptr);
  }

  void restoreState(int version, IStream *stream) override {
    if (version > Version) {
      return;
    }
    stream->Read(_params, sizeof(_params), nullptr);
  }

  void FLAPI GetName(int section, int index, int value, char *name) override;
  int FLAPI ProcessEvent(int eventId, int value, int flags) override;
  int FLAPI ProcessParam(int index, int value, int recflags) override;

  void FLAPI Gen_Render(PWAV32FS buffer, int &length) override;
  //void FLAPI Eff_Render(PWAV32FS src, PWAV32FS dst, int length) override;

  TVoiceHandle FLAPI TriggerVoice(PVoiceParams params, intptr_t tag) override;
  void FLAPI Voice_Release(TVoiceHandle handle) override;
  void FLAPI Voice_Kill(TVoiceHandle handle) override;
};

A8_DECLARE_FL_PLUGIN(FLSaw)

void FLAPI FLSaw::GetName(int section, int index, int value, char *name) {
  const char *s = nullptr;

  switch (section) {
    case FPN_Param:
      switch (index) {
        case ParamId::Volume:
          s = "Volume";
          break;
        case ParamId::WaveTable:
          s = "Wave Table";
          break;
      }
      break;
  }

  if (s) {
    // name is at least 256 bytes.
    strcpy_s(name, 256, s);
  }
}

int FLAPI FLSaw::ProcessEvent(int eventId, int value, int flags) {
  return value;
}

int FLAPI FLSaw::ProcessParam(int index, int value, int recflags) {
  auto fn = [recflags](int flag, const char *name) -> const char * {
    if (recflags & flag) return name;
    return "";
  };
  Log::debug("ProcessParam({}, {}, 0x{:X} ={}{}{}{}{}{}{}{})",
             index, value, recflags,
             fn(REC_GetValue,      " GetValue"),
             fn(REC_UpdateValue,   " UpdateValue"),
             fn(REC_UpdateControl, " UpdateControl"),
             fn(REC_InternalCtrl,  " InternalCtrl"),
             fn(REC_FromMIDI,      " FromMIDI"),
             fn(REC_NoLink,        " NoLink"),
             fn(REC_PlugReserved,  " PlugReserved"),
             fn(REC_ShowHint,      " ShowHint"));

  if (recflags & REC_FromMIDI) {
    value = value * 100 / 65536;
  }

  if (recflags & REC_GetValue) {
    value = _params[index];
  }

  if (recflags & REC_UpdateValue) {
    _params[index] = value;
  }

  if (recflags & REC_UpdateControl) {
    // Update GUI control.
  }

  return value;
}

void FLAPI FLSaw::Gen_Render(PWAV32FS destBuffer, int &length) {
  float volume = _params[ParamId::Volume] / 100.f;
  float lvol, rvol;

  PWAV32FM buffer = (PWAV32FM)PlugHost->TempBuffers[0];
  PWaveT sawTable = PlugHost->WaveTables[3];
  //PWaveT sawTable = PlugHost->WaveTables[_params[ParamId::WaveTable]];

  for (auto &&voice : _voices) {
    if (!voice) {
      continue;
    }
    if (voice.IsReleased) {
      lvol = 0;
      rvol = 0;
    } else {
      PlugHost->ComputeLRVol(lvol, rvol, voice.Params->FinalLevels.Pan,
                             voice.Params->FinalLevels.Vol);
    }

    if (voice.Params->FinalLevels.Pitch != voice.CurrentPitch) {
      voice.CurrentPitch = voice.Params->FinalLevels.Pitch;
      voice.Speed = (int)GetStep_Cents(voice.CurrentPitch);
    }

    for (int i = 0; i < length; ++i) {
      int pos = voice.Position >> WaveT_Shift;
      buffer[i] = (*sawTable)[pos] * volume;
      // May overflow
      voice.Position += static_cast<unsigned>(voice.Speed);
    }

    PlugHost->AddWave_32FM_32FS_Ramp(buffer, destBuffer, length, lvol, rvol,
                                     voice.LastLVol, voice.LastRVol);

    if (voice.IsReleased) {
      PlugHost->Voice_Kill(voice.HostTag, TRUE);
    }
  }
}

//void FLAPI FLSaw::Eff_Render(PWAV32FS src, PWAV32FS dst, int length) {
//}

TVoiceHandle FLAPI FLSaw::TriggerVoice(PVoiceParams params, intptr_t tag) {
  auto index = nextVoice();
  if (index == -1) {
    return 0;
  }

  auto &voice = _voices[index];

  voice.HostTag = tag;
  voice.Params = params;
  voice.IsReleased = false;
  voice.CurrentPitch = voice.Params->FinalLevels.Pitch;
  voice.Speed = (int)GetStep_Cents(voice.CurrentPitch);

  return (TVoiceHandle)&voice;
}

void FLAPI FLSaw::Voice_Release(TVoiceHandle handle) {
  if (!handle) {
    return;
  }
  ((TVoice *)handle)->IsReleased = true;
}

void FLAPI FLSaw::Voice_Kill(TVoiceHandle handle) {
  if (!handle) {
    return;
  }
  ((TVoice *)handle)->clear();
}
