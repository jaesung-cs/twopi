#ifndef TWOPI_WINDOW_EVENT_KEYBOARD_EVENT_H_
#define TWOPI_WINDOW_EVENT_KEYBOARD_EVENT_H_

#include <twopi/window/event/event.h>
#include <twopi/window/event/types.h>

#include <memory>

namespace twopi
{
namespace window
{
class KeyboardEvent : public Event
{
public:
  KeyboardEvent();

  KeyboardEvent(int key, KeyState state);

  virtual ~KeyboardEvent();

  int Key() const;
  KeyState State() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_WINDOW_EVENT_KEYBOARD_EVENT_H_
