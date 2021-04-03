#ifndef TWOPI_GPU_GPU_SWAPCHAIN_H_
#define TWOPI_GPU_GPU_SWAPCHAIN_H_

#include <utility>
#include <memory>
#include <vector>

namespace vk
{
enum class Result;
enum class Format;
class PhysicalDevice;
class Device;
class Semaphore;
class ImageView;
}

namespace twopi
{
namespace gpu
{
class Device;

class Swapchain
{
public:
  Swapchain() = delete;
  Swapchain(std::shared_ptr<Device> device, uint32_t width, uint32_t height);
  ~Swapchain();

  const std::vector<vk::ImageView>& ImageViews() const;
  uint32_t ImageCount() const;
  vk::Format Format() const;

  void Resize(uint32_t width, uint32_t height);

  std::pair<uint32_t, vk::Result> AcquireNextImage(vk::Semaphore semaphore);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_GPU_GPU_SWAPCHAIN_H_
