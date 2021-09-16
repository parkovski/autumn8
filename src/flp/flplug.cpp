#define A8_INCLUDE_PLATFORM
#include "a8/a8.h"

#include "a8/flp/flplug.h"
#include "a8/gfx/views/webview.h"

#include <spdlog/sinks/basic_file_sink.h>

#pragma comment(lib, "mincore.lib")

using namespace a8::flp;

constexpr int PlugVersion = 1;
static TFruityPlugInfo gs_PlugInfo = {
  CurrentSDKVersion,
  "A8Saw",
  "A8Saw",
  FPF_Type_FullGen,
  FLPlugin::Param::count,
  FLPlugin::MaxVoices,
  0 // Internal controllers
};

std::shared_ptr<spdlog::logger> a8::Log::g_logger;

extern "C" __declspec(dllexport) TFruityPlug * __stdcall
CreatePlugInstance(TFruityPlugHost *Host, int Tag) {
  char x[sizeof(a8::Log)];
  new (x) a8::Log(spdlog::basic_logger_mt("A8", "C:\\Users\\parker\\a8fl.txt"));
  spdlog::set_level(spdlog::level::trace);
  spdlog::set_pattern(a8::Log::pattern_no_color_explicit);
  spdlog::flush_on(spdlog::level::trace);

  a8::Log::info("In CreatePlugInstance: Host=0x{:X}; Tag={}", (size_t)Host, Tag);
  a8::platform::g_platformInit.init(HInstance);
  a8::Log::info("Platform init finished.");

  return new FLPlugin(Host, Tag);
}

BOOL APIENTRY DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID) {
  switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
      HInstance = hInstDLL;
      break;
  }

  return TRUE;
}

FLPlugin::FLPlugin(TFruityPlugHost *host, int tag)
  : TCPPFruityPlug(tag, host, HInstance)
{
  Info = &gs_PlugInfo;
  _params[Param::volume] = 80;
}

FLPlugin::~FLPlugin() {
  if (_window) {
    _window->close();
  }
}

intptr_t __stdcall FLPlugin::Dispatcher(intptr_t id, intptr_t index, intptr_t value) {
  if (auto r = TCPPFruityPlug::Dispatcher(id, index, value)) {
    return r;
  }

  switch (id) {
    case FPD_ShowEditor:
      if (value) {
        using a8::gfx::SystemEvent;
        _window = new a8::gfx::Window();
        _window->resize(0, 0, 400, 400);
        _window->setView(new a8::gfx::WebView);
        _window->open("A8.FLP.Saw", (void *)value);
        EditorHandle = (HWND)_window->nativeHandle();
      } else {
        _window->close();
        _window = nullptr;
        EditorHandle = nullptr;
      }
      break;

    case FPD_SetSampleRate:
      AudioRenderer.setSmpRate((int)value);
      PitchMul = (float)MiddleCMul / value;
      break;

    default:
      break;
  }

  return 0;
}

void __stdcall FLPlugin::SaveRestoreState(IStream *stream, BOOL save) {
  if (save) {
    stream->Write(&PlugVersion, sizeof(int), nullptr);
    stream->Write(_params, sizeof(_params), nullptr);
  } else {
    int version = std::numeric_limits<int>::max();
    stream->Read(&version, sizeof(int), nullptr);
    if (version <= PlugVersion) {
      stream->Read(_params, sizeof(_params), nullptr);
    }
  }
}

void __stdcall FLPlugin::GetName(int section, int index, int value, char *name) {
  if (section == FPN_Param && index == Param::volume) {
    // name is at least 256 bytes.
    memcpy(name, "Volume", 7);
  }
}

int __stdcall FLPlugin::ProcessEvent(int eventId, int value, int flags) {
  return value;
}

int __stdcall FLPlugin::ProcessParam(int index, int value, int recflags) {
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

void __stdcall FLPlugin::Gen_Render(PWAV32FS destBuffer, int &length) {
  float volume = _params[Param::volume] / 100.f;
  float lvol, rvol;

  PWAV32FM buffer = (PWAV32FM)PlugHost->TempBuffers[0];
  PWaveT sawTable = PlugHost->WaveTables[3];

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

//void __stdcall FLPlugin::Eff_Render(PWAV32FS src, PWAV32FS dst, int length) {
//}

TVoiceHandle __stdcall FLPlugin::TriggerVoice(PVoiceParams params, intptr_t tag) {
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

void __stdcall FLPlugin::Voice_Release(TVoiceHandle handle) {
  if (!handle) {
    return;
  }
  ((TVoice *)handle)->IsReleased = true;
}

void __stdcall FLPlugin::Voice_Kill(TVoiceHandle handle) {
  if (!handle) {
    return;
  }
  ((TVoice *)handle)->clear();
}

int __stdcall FLPlugin::Voice_ProcessEvent(TVoiceHandle handle, int eventId, int value, int flags) {
  return 0;
}
