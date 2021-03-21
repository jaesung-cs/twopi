#ifndef TWOPI_WINDOW_EVENT_MOUSE_WHEEL_EVENT_H_
#define TWOPI_WINDOW_EVENT_MOUSE_WHEEL_EVENT_H_

#include <twopi/window/event/event.h>

#include <memory>

namespace twopi
{
namespace window
{
namespace impl
{
class MouseWheelEventImpl;
}

class MouseWheelEvent : public Event
{
public:
  MouseWheelEvent() = delete;

  explicit MouseWheelEvent(int scroll);

  ~MouseWheelEvent() override;

  int Scroll() const;

private:
  std::unique_ptr<impl::MouseWheelEventImpl> impl_;
};
}
}

#endif // TWOPI_WINDOW_EVENT_MOUSE_WHEEL_EVENT_H_
