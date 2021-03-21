#include <twopi/window/event/resize_event.h>

namespace twopi
{
namespace window
{
namespace impl
{
class ResizeEventImpl
{
public:
  ResizeEventImpl() = delete;

  ResizeEventImpl(int width, int height)
    : width_(width), height_(height)
  {
  }

  ~ResizeEventImpl() = default;

  auto Width() const { return width_; }
  auto Height() const { return height_; }

private:
  int width_ = 1;
  int height_ = 1;
};
}

ResizeEvent::ResizeEvent(int width, int height)
  : Event()
{
  impl_ = std::make_unique<impl::ResizeEventImpl>(width, height);
}

ResizeEvent::~ResizeEvent() = default;

int ResizeEvent::Width() const
{
  return impl_->Width();
}

int ResizeEvent::Height() const
{
  return impl_->Height();
}
}
}
