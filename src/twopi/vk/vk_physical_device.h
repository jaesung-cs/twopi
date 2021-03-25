#ifndef TWOPI_VK_VK_PHYSICAL_DEVICE_H_
#define TWOPI_VK_VK_PHYSICAL_DEVICE_H_

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

namespace twopi
{
namespace vk
{
class PhysicalDevice
{
public:
  PhysicalDevice() = delete;
  PhysicalDevice(VkInstance instance, VkPhysicalDevice physical_device);

  PhysicalDevice(const PhysicalDevice& rhs);
  PhysicalDevice& operator = (const PhysicalDevice& rhs);

  PhysicalDevice(PhysicalDevice&& rhs) noexcept;
  PhysicalDevice& operator = (PhysicalDevice&& rhs) noexcept;

  ~PhysicalDevice();

  operator VkPhysicalDevice() const;

  const VkPhysicalDeviceProperties& Properties() const;
  const VkPhysicalDeviceFeatures& Features() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_VK_VK_PHYSICAL_DEVICE_H_
