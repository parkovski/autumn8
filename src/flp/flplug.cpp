#define A8_INCLUDE_PLATFORM
#include "a8/a8.h"

#include "a8/flp/flplug.h"
#include "a8/util/utfconv.h"

#include <spdlog/sinks/basic_file_sink.h>

#pragma comment(lib, "mincore.lib")

using namespace a8::flp;

static struct A8FLPlugInfo {
  TFruityPlugInfo fruityPlugInfo{};
  TFruityPlugHost *host = nullptr;
  int hostTag = 0;
  int version = 0;
  char moduleName[64];
} gs_PlugInfo;

std::shared_ptr<spdlog::logger> a8::Log::g_logger;

extern "C" __declspec(dllexport)
TFruityPlug * FLAPI
CreatePlugInstance(TFruityPlugHost *Host, int Tag) {
  char x[sizeof(a8::Log)];
  new (x) a8::Log(spdlog::basic_logger_mt("A8", "C:\\Users\\parker\\a8fl.txt"));
  spdlog::set_level(spdlog::level::trace);
  spdlog::set_pattern(a8::Log::pattern_no_color_explicit);
  spdlog::flush_on(spdlog::level::trace);

  a8::Log::info("In CreatePlugInstance: Host=0x{:X}; Tag={}", (size_t)Host, Tag);
  a8::platform::g_platformInit.init(HInstance);
  a8::Log::info("Platform init finished.");

  gs_PlugInfo.host = Host;
  gs_PlugInfo.hostTag = Tag;

  wchar_t modulePath[MAX_PATH];
  auto length = GetModuleFileName(HInstance, modulePath, MAX_PATH);
  if (length && length < MAX_PATH) {
    // See https://stackoverflow.com/a/28056007
    // Basically GetModuleFileName is wack. We'll copy the short name over
    // later if there are problems.
    auto u8path = a8::to_utf8(modulePath);
    size_t u8length = u8path.length();
    size_t nameStart = 0;
    size_t extStart = u8length;
    // This looks weird but unsigned math wraps, so when this gets to -1 it
    // compares greater than the length.
    for (size_t i = u8length - 2; i < u8length; --i) {
      char c = u8path[i];
      if (c == '/' || c == '\\') {
        nameStart = i + 1;
        break;
      }
      if (c == '.' && extStart == u8length) {
        extStart = i;
        if (i > 5 && u8path[i - 4] == '_' && (u8path[i - 3] | 0x20) == 'x' &&
            u8path[i - 2] == '6' && u8path[i - 1] == '4') {
          extStart -= 4;
        }
      }
    }

    u8length = extStart - nameStart;
    if (u8length >= sizeof(gs_PlugInfo.moduleName) - 1) {
      u8length = sizeof(gs_PlugInfo.moduleName) - 2;
    }
    memcpy(gs_PlugInfo.moduleName, &u8path[nameStart], u8length);
    gs_PlugInfo.moduleName[u8length + 1] = 0;
  } else {
    gs_PlugInfo.moduleName[0] = 0;
  }

  return FLPlugin::create(gs_PlugInfo.moduleName);
}

BOOL APIENTRY DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID) {
  switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
      HInstance = hInstDLL;
      break;
  }

  return TRUE;
}

void FLPlugin::fillPlugInfo(int version, const char *name, int flags,
                            int params, int polyVoices, int outControls,
                            int outVoices) noexcept {
  if (gs_PlugInfo.fruityPlugInfo.ShortName) {
    Log::warn("Plugin info already initialized");
  }

  assert(version > 0 &&
         name && name[0] &&
         flags &&
         params >= 0 &&
         polyVoices > 0 &&
         outControls >= 0 &&
         outVoices >= 0);

  gs_PlugInfo.version = version;
  gs_PlugInfo.fruityPlugInfo.SDKVersion = CurrentSDKVersion;
  gs_PlugInfo.fruityPlugInfo.ShortName = const_cast<char *>(name);
  gs_PlugInfo.fruityPlugInfo.Flags = flags;
  gs_PlugInfo.fruityPlugInfo.NumParams = params;
  gs_PlugInfo.fruityPlugInfo.DefPoly = polyVoices;
  gs_PlugInfo.fruityPlugInfo.NumOutCtrls = outControls;
  gs_PlugInfo.fruityPlugInfo.NumOutVoices = outVoices;

  if (!gs_PlugInfo.moduleName[0]) {
    // Something went wrong, just use the short name.
    gs_PlugInfo.fruityPlugInfo.LongName = const_cast<char *>(name);
    Log::warn("Plugin '{}' long name missing; using short name.", name);
  } else {
    gs_PlugInfo.fruityPlugInfo.LongName = gs_PlugInfo.moduleName;
    Log::debug("Plugin '{}' (module {}) info initialized.", name,
               gs_PlugInfo.moduleName);
  }
}

FLPlugin::FLPlugin()
  : TCPPFruityPlug(gs_PlugInfo.hostTag, gs_PlugInfo.host, HInstance)
{
  Info = &gs_PlugInfo.fruityPlugInfo;
}

FLPlugin::~FLPlugin() {
  if (_window) {
    _window->close();
  }
}

intptr_t FLAPI FLPlugin::Dispatcher(intptr_t id, intptr_t index, intptr_t value) {
  if (auto r = TCPPFruityPlug::Dispatcher(id, index, value)) {
    return r;
  }

  switch (id) {
    case FPD_ShowEditor:
      if (value) {
        _window = createEditor();
        _window->open(gs_PlugInfo.fruityPlugInfo.ShortName, (void *)value);
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

void FLAPI FLPlugin::SaveRestoreState(IStream *stream, BOOL save) {
  if (save) {
    stream->Write(&gs_PlugInfo.version, sizeof(int), nullptr);
    saveState(stream);
  } else {
    int version = std::numeric_limits<int>::max();
    stream->Read(&version, sizeof(int), nullptr);
    restoreState(version, stream);
  }
}
