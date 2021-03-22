#include <twopi/window/win32_window.h>

#include <memory>

namespace twopi
{
namespace window
{
class Win32Window::Impl
{
public:
  Impl() = default;

  ~Impl() = default;

private:
};

Win32Window::Win32Window()
  : Window()
{
  impl_ = std::make_unique<Impl>();
}

Win32Window::~Win32Window() = default;
}
}
