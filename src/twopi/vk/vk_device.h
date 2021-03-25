#ifndef TWOPI_VK_VK_DEVICE_H_
#define TWOPI_VK_VK_DEVICE_H_

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

namespace twopi
{
namespace vk
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
    class Impl;
    std::unique_ptr<Impl> impl_;
  };

public:
  Device();
  Device(VkPhysicalDevice physical_device, VkDevice device);

  Device(const Device& rhs);
  Device& operator = (const Device& rhs);

  Device(Device&& rhs) noexcept;
  Device& operator = (Device&& rhs) noexcept;

  ~Device();

  operator VkDevice() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_VK_VK_DEVICE_H_
