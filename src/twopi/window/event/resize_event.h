#ifndef TWOPI_WINDOW_EVENT_RESIZE_EVENT_H_
#define TWOPI_WINDOW_EVENT_RESIZE_EVENT_H_

#include <twopi/window/event/event.h>

#include <memory>

namespace twopi
{
namespace window
{
namespace impl
{
class ResizeEventImpl;
}

class ResizeEvent : public Event
{
public:
  ResizeEvent() = delete;

  ResizeEvent(int width, int height);

  ~ResizeEvent() override;

  int Width() const;
  int Height() const;

private:
  std::unique_ptr<impl::ResizeEventImpl> impl_;
};
}
}

#endif // TWOPI_WINDOW_EVENT_RESIZE_EVENT_H_
