#ifndef A8_GFX_DELEGATE_H_
#define A8_GFX_DELEGATE_H_

#include "a8/gfx/events.h"

namespace a8::gfx {

class View;
class Renderer;

class EventDelegate {
protected:
  explicit EventDelegate() noexcept {}

public:
  virtual ~EventDelegate() = 0;

  virtual bool on(View &, const PointerEvent &)      { return false; }
  virtual bool on(View &, const KeyboardEvent &)     { return false; }
  virtual bool on(View &, const GamepadEvent &)      { return false; }
  virtual bool on(View &, const MidiEvent &)         { return false; }
  virtual bool on(View &, const SizingEvent &)       { return false; }
  virtual bool on(View &, const MovingEvent &)       { return false; }
  virtual bool on(View &, const ResizeEvent &)       { return false; }
  virtual bool on(View &, const DisplayScaleEvent &) { return false; }
  virtual bool on(View &, const SystemEvent &)       { return false; }
  virtual bool on(View &, const StyleChangeEvent &)  { return false; }
  virtual bool on(View &, const DragDropEvent &)     { return false; }
  virtual bool on(View &, const DataEvent &)         { return false; }

  // Return true to indicate all events were handled.
  static class EmptyDelegate final : public EventDelegate {
    bool on(View &, const PointerEvent &) override      { return true; }
    bool on(View &, const KeyboardEvent &) override     { return true; }
    bool on(View &, const GamepadEvent &) override      { return true; }
    bool on(View &, const MidiEvent &) override         { return true; }
    bool on(View &, const SizingEvent &) override       { return true; }
    bool on(View &, const MovingEvent &) override       { return true; }
    bool on(View &, const ResizeEvent &) override       { return true; }
    bool on(View &, const DisplayScaleEvent &) override { return true; }
    bool on(View &, const SystemEvent &) override       { return true; }
    bool on(View &, const StyleChangeEvent &) override  { return true; }
    bool on(View &, const DragDropEvent &) override     { return true; }
    bool on(View &, const DataEvent &) override         { return true; }
  } Empty;
};

class RenderDelegate {
protected:
  explicit RenderDelegate() noexcept {}

public:
  virtual ~RenderDelegate() = 0;

  virtual void render(View &, Renderer &) = 0;

  static class EmptyDelegate final : public RenderDelegate {
  public:
    void render(View &, Render &) override;
  } Empty;
};

} // namespace a8::gfx

#endif /* A8_GFX_DELEGATE_H_ */

