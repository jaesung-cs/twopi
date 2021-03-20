#ifndef TWOPI_WINDOW_EVENT_KEYBOARD_EVENT_H_
#define TWOPI_WINDOW_EVENT_KEYBOARD_EVENT_H_

#include <twopi/window/event/event.h>
#include <twopi/window/event/types.h>

#include <memory>

namespace twopi
{
namespace window
{
namespace impl
{
class KeyboardEventImpl;
}

class KeyboardEvent : public Event
{
public:
  KeyboardEvent();

  KeyboardEvent(int key, KeyState state);

  virtual ~KeyboardEvent();

  int Key() const;
  KeyState State() const;

private:
  std::unique_ptr<impl::KeyboardEventImpl> impl_;
};
}
}

#endif // TWOPI_WINDOW_EVENT_KEYBOARD_EVENT_H_
