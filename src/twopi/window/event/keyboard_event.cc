#include <twopi/window/event/keyboard_event.h>

#include <type_traits>
#include <array>

namespace twopi
{
namespace window
{
class KeyboardEvent::Impl
{
public:
  Impl()
  {
  }

  Impl(int key, KeyState state)
    : key_(key), state_(state)
  {
  }

  ~Impl() = default;

  auto Key() const
  {
    return key_;
  }

  auto State() const
  {
    return state_;
  }

private:
  int key_ = 0;
  KeyState state_ = KeyState::UNDEFINED;
};

KeyboardEvent::KeyboardEvent()
  : Event()
{
  impl_ = std::make_unique<Impl>();
}

KeyboardEvent::KeyboardEvent(int key, KeyState state)
  : Event()
{
  impl_ = std::make_unique<Impl>(key, state);
}

KeyboardEvent::~KeyboardEvent() = default;

int KeyboardEvent::Key() const
{
  return impl_->Key();
}

KeyState KeyboardEvent::State() const
{
  return impl_->State();
}
}
}
