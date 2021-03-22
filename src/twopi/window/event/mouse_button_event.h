#ifndef TWOPI_WINDOW_EVENT_MOUSE_BUTTON_EVENT_H_
#define TWOPI_WINDOW_EVENT_MOUSE_BUTTON_EVENT_H_

#include <twopi/window/event/event.h>
#include <twopi/window/event/types.h>

#include <memory>

namespace twopi
{
namespace window
{
class MouseButtonEvent : public Event
{
public:
  MouseButtonEvent() = delete;

  MouseButtonEvent(MouseButton button, MouseButtonState state, int x, int y);

  ~MouseButtonEvent() override;

  MouseButton Button() const;
  MouseButtonState State() const;

  int X() const;
  int Y() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_WINDOW_EVENT_MOUSE_BUTTON_EVENT_H_
