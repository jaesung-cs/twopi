#ifndef TWOPI_GPU_GPU_DEVICE_H_
#define TWOPI_GPU_GPU_DEVICE_H_

#include <vector>
#include <memory>

namespace vk
{
class Instance;
class Device;
class SurfaceKHR;
class PhysicalDevice;
}

namespace twopi
{
namespace gpu
{
class Device
{
public:
  Device() = delete;
  Device(vk::Instance instance, vk::SurfaceKHR surface);
  ~Device();

  vk::PhysicalDevice PhysicalDevice() const;
  vk::Device DeviceHandle() const;
  vk::SurfaceKHR Surface() const;
  std::vector<uint32_t> QueueFamilies() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_GPU_GPU_DEVICE_H_
