#include <twopi/window/event/mouse_move_event.h>

namespace twopi
{
namespace window
{
namespace impl
{
class MouseMoveEventImpl
{
public:
  MouseMoveEventImpl() = delete;

  MouseMoveEventImpl(int x, int y)
    : x_(x), y_(y)
  {
  }

  ~MouseMoveEventImpl() = default;

  auto X() const { return x_; }
  auto Y() const { return y_; }

private:
  int x_ = 0;
  int y_ = 0;
};
}

MouseMoveEvent::MouseMoveEvent(int x, int y)
  : Event()
{
  impl_ = std::make_unique<impl::MouseMoveEventImpl>(x, y);
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
