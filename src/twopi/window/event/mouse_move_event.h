#ifndef TWOPI_WINDOW_EVENT_MOUSE_MOVE_EVENT_H_
#define TWOPI_WINDOW_EVENT_MOUSE_MOVE_EVENT_H_

#include <twopi/window/event/event.h>

#include <memory>

namespace twopi
{
namespace window
{
namespace impl
{
class MouseMoveEventImpl;
}

class MouseMoveEvent : public Event
{
public:
  MouseMoveEvent() = delete;

  MouseMoveEvent(int x, int y);

  ~MouseMoveEvent() override;

  int X() const;
  int Y() const;

private:
  std::unique_ptr<impl::MouseMoveEventImpl> impl_;
};
}
}

#endif // TWOPI_WINDOW_EVENT_MOUSE_MOVE_EVENT_H_
