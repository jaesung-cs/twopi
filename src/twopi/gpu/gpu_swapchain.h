#ifndef TWOPI_GPU_GPU_SWAPCHAIN_H_
#define TWOPI_GPU_GPU_SWAPCHAIN_H_

#include <memory>

namespace twopi
{
namespace gpu
{
class Swapchain
{
public:
  Swapchain() = delete;
  Swapchain(int width, int height);
  ~Swapchain();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_GPU_GPU_SWAPCHAIN_H_
