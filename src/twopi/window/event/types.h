#ifndef TWOPI_WINDOW_EVENT_TYPES_H_
#define TWOPI_WINDOW_EVENT_TYPES_H_

#include <cstdint>

namespace twopi
{
namespace window
{
enum class MouseButton : uint8_t
{
  LEFT,
  RIGHT,
  MIDDLE,
  UNDEFINED,
};

enum class MouseButtonState : uint8_t
{
  RELEASED,
  PRESSED,
  UNDEFINED,
};

enum class KeyState : uint8_t
{
  RELEASED,
  PRESSED,
  UNDEFINED,
};
}
}

#endif // TWOPI_WINDOW_EVENT_TYPES_H_
