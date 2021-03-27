#ifndef TWOPI_VK_VK_PHYSICAL_DEVICE_H_
#define TWOPI_VK_VK_PHYSICAL_DEVICE_H_

#include <memory>
#include <vector>

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class PhysicalDevice
{
public:
  PhysicalDevice();
  explicit PhysicalDevice(vk::PhysicalDevice physical_device);

  ~PhysicalDevice();

  operator vk::PhysicalDevice() const;

  std::vector<vk::ExtensionProperties> Extensions() const;
  vk::PhysicalDeviceProperties Properties() const;
  vk::PhysicalDeviceProperties2 Properties2() const;
  vk::PhysicalDeviceFeatures Features() const;

private:
  vk::PhysicalDevice physical_device_;
};
}
}

#endif // TWOPI_VK_VK_PHYSICAL_DEVICE_H_
