#ifndef TWOPI_WINDOW_WIN32_WINDOW_H_
#define TWOPI_WINDOW_WIN32_WINDOW_H_

#include <twopi/window/window.h>

#include <memory>

namespace twopi
{
namespace window
{
namespace impl
{
class Win32WindowImpl;
}

class Win32Window : public Window
{
public:
  Win32Window();
  ~Win32Window() override;

private:
  std::unique_ptr<impl::Win32WindowImpl> impl_;
};
}
}

#endif // TWOPI_WINDOW_WIN32_WINDOW_H_
