#ifndef TWOPI_VK_VK_DEVICE_H_
#define TWOPI_VK_VK_DEVICE_H_

#include <memory>
#include <vector>

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class PhysicalDevice;

class Device
{
public:
  class Creator
  {
  public:
    Creator() = delete;
    explicit Creator(PhysicalDevice physical_device);
    ~Creator();

    Device Create();

  private:
    vk::DeviceCreateInfo create_info_{};
    vk::DeviceQueueCreateInfo queue_create_info_{};
    float queue_priority_ = 1.f;
  };

public:
  Device();
  Device(vk::Device device);

  ~Device();

  void Destroy();

  operator vk::Device() const;

private:
  vk::Device device_;
};
}
}

#endif // TWOPI_VK_VK_DEVICE_H_
