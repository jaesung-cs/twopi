#include <twopi/window/event/mouse_button_event.h>

namespace twopi
{
namespace window
{
class MouseButtonEvent::Impl
{
public:
  Impl() = delete;

  Impl(MouseButton button, MouseButtonState state, int x, int y)
    : button_(button), state_(state), x_(x), y_(y)
  {
  }

  ~Impl() = default;

  MouseButton Button() const
  {
    return button_;
  }

  MouseButtonState State() const
  {
    return state_;
  }

  auto X() const { return x_; }
  auto Y() const { return y_; }

private:
  MouseButton button_ = MouseButton::UNDEFINED;
  MouseButtonState state_ = MouseButtonState::UNDEFINED;
  int x_ = 0;
  int y_ = 0;
};

MouseButtonEvent::MouseButtonEvent(MouseButton button, MouseButtonState state, int x, int y)
  : Event()
{
  impl_ = std::make_unique<Impl>(button, state, x, y);
}

MouseButtonEvent::~MouseButtonEvent() = default;

MouseButton MouseButtonEvent::Button() const
{
  return impl_->Button();
}

MouseButtonState MouseButtonEvent::State() const
{
  return impl_->State();
}

int MouseButtonEvent::X() const
{
  return impl_->X();
}

int MouseButtonEvent::Y() const
{
  return impl_->Y();
}
}
}
