#ifndef TWOPI_WINDOW_EVENT_MOUSE_BUTTON_EVENT_H_
#define TWOPI_WINDOW_EVENT_MOUSE_BUTTON_EVENT_H_

#include <twopi/window/event/event.h>
#include <twopi/window/event/types.h>

#include <memory>

namespace twopi
{
namespace window
{
namespace impl
{
class MouseButtonEventImpl;
}

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
  std::unique_ptr<impl::MouseButtonEventImpl> impl_;
};
}
}

#endif // TWOPI_WINDOW_EVENT_MOUSE_BUTTON_EVENT_H_
