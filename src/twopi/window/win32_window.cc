#include <twopi/window/win32_window.h>

#include <memory>

namespace twopi
{
namespace window
{
namespace impl
{
class Win32WindowImpl
{
public:
  Win32WindowImpl() = default;

  ~Win32WindowImpl() = default;

private:
};
}

Win32Window::Win32Window()
  : Window()
{
  impl_ = std::make_unique<impl::Win32WindowImpl>();
}

Win32Window::~Win32Window() = default;
}
}
