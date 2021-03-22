#include <twopi/window/event/mouse_wheel_event.h>

namespace twopi
{
namespace window
{
class MouseWheelEvent::Impl
{
public:
  Impl() = delete;

  Impl(int scroll)
    : scroll_(scroll)
  {
  }

  ~Impl() = default;

  auto Scroll() const { return scroll_; }

private:
  int scroll_ = 0;
};

MouseWheelEvent::MouseWheelEvent(int scroll)
  : Event()
{
  impl_ = std::make_unique<Impl>(scroll);
}

MouseWheelEvent::~MouseWheelEvent() = default;

int MouseWheelEvent::Scroll() const
{
  return impl_->Scroll();
}
}
}
