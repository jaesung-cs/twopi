#include <twopi/window/window.h>

namespace twopi
{
namespace window
{
class Window::Impl
{
public:
  Impl() = default;

  ~Impl() = default;

  auto Width() const { return width_; }
  auto Height() const { return height_; }

  void Resized(int width, int height)
  {
    width_ = width;
    height_ = height;
  }

  auto MaxWidth() const { return 1920; }
  auto MaxHeight() const { return 1080; }

private:
  int width_ = 1600;
  int height_ = 900;
};

Window::Window()
{
  impl_ = std::make_unique<Impl>();
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

int Window::MaxWidth() const
{
  return impl_->MaxWidth();
}

int Window::MaxHeight() const
{
  return impl_->MaxHeight();
}

}
}
