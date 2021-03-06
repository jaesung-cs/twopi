#include <twopi/window/event/mouse_move_event.h>

namespace twopi
{
namespace window
{
class MouseMoveEvent::Impl
{
public:
  Impl() = delete;

  Impl(int x, int y)
    : x_(x), y_(y)
  {
  }

  ~Impl() = default;

  auto X() const { return x_; }
  auto Y() const { return y_; }

private:
  int x_ = 0;
  int y_ = 0;
};

MouseMoveEvent::MouseMoveEvent(int x, int y)
  : Event()
{
  impl_ = std::make_unique<Impl>(x, y);
}

MouseMoveEvent::~MouseMoveEvent() = default;

int MouseMoveEvent::X() const
{
  return impl_->X();
}

int MouseMoveEvent::Y() const
{
  return impl_->Y();
}
}
}
