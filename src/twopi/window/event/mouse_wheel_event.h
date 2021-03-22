#ifndef TWOPI_WINDOW_EVENT_MOUSE_WHEEL_EVENT_H_
#define TWOPI_WINDOW_EVENT_MOUSE_WHEEL_EVENT_H_

#include <twopi/window/event/event.h>

#include <memory>

namespace twopi
{
namespace window
{
class MouseWheelEvent : public Event
{
public:
  MouseWheelEvent() = delete;

  explicit MouseWheelEvent(int scroll);

  ~MouseWheelEvent() override;

  int Scroll() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_WINDOW_EVENT_MOUSE_WHEEL_EVENT_H_
