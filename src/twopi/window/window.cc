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

  auto Width() const { return width_; }
  auto Height() const { return height_; }

  void Resized(int width, int height)
  {
    width_ = width;
    height_ = height;
  }

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

void Window::Resized(int width, int height)
{
  impl_->Resized(width, height);
}
}
}
