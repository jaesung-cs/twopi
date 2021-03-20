#include <twopi/window/window.h>

namespace twopi
{
namespace window
{
namespace impl
{
class WindowImpl
{
public:
  WindowImpl() = default;

  ~WindowImpl() = default;

  const auto Width() const { return width_; }
  const auto Height() const { return height_; }

private:
  int width_ = 1600;
  int height_ = 900;
};
}

Window::Window()
{
  impl_ = std::make_unique<impl::WindowImpl>();
}

Window::~Window() = default;

int Window::Width() const
{
  return impl_->Width();
}

int Window::Height() const
{
  return impl_->Height();
}
}
}
