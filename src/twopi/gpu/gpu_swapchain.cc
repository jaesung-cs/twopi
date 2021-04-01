#include <twopi/gpu/gpu_swapchain.h>

namespace twopi
{
namespace gpu
{
class Swapchain::Impl
{
public:
  Impl() = delete;

  Impl(int width, int height)
  {
  }

  ~Impl() = default;

private:
};

Swapchain::Swapchain(int width, int height)
  : impl_(std::make_unique<Impl>(width, height))
{
}

Swapchain::~Swapchain() = default;
}
}
