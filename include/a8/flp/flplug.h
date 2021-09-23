#pragma once

#include "fp_cplug.h"

#include "a8/gfx/window.h"

#define A8_DECLARE_FL_PLUGIN(Type) \
  ::a8::flp::FLPlugin *::a8::flp::FLPlugin::create(std::string_view) { \
    return ::a8::flp::FLPlugin::createPlugin<Type>(); \
  }

#define FLAPI __stdcall

namespace a8::flp {

class FLPlugin : public TCPPFruityPlug {
  static void fillPlugInfo(int version, const char *name, int flags,
                           int params, int polyVoices, int outControls,
                           int outVoices) noexcept;

protected:
  a8::gfx::Window *_window = nullptr;

public:
  // Name contains the module filename, without the path, extension, or
  // optional "_x64" suffix.
  static FLPlugin *create(std::string_view name);

  explicit FLPlugin();
  ~FLPlugin();

  virtual a8::gfx::Window *createEditor() = 0;
  virtual void saveState(IStream *stream) = 0;
  virtual void restoreState(int version, IStream *stream) = 0;

  intptr_t FLAPI Dispatcher(intptr_t id, intptr_t index, intptr_t value) override;
  void FLAPI SaveRestoreState(IStream *stream, BOOL save) override;

  //int FLAPI Voice_ProcessEvent(TVoiceHandle handle, int eventId, int value, int flags);

private:
  template<class TPlugin>
  static TPlugin *createPlugin() {
    fillPlugInfo(
      TPlugin::Version,
      TPlugin::PlugName,
      TPlugin::Flags,
      TPlugin::DefaultParamCount,
      TPlugin::DefaultPolyVoices,
      TPlugin::OutControlCount,
      TPlugin::OutVoiceCount
    );

    return new TPlugin;
  }
};

} // namespace a8::flp
