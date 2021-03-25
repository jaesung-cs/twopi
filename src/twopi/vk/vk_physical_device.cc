#include <twopi/vk/vk_physical_device.h>

namespace twopi
{
namespace vkw
{
PhysicalDevice::PhysicalDevice(vk::PhysicalDevice physical_device)
  : physical_device_(physical_device)
{
}

PhysicalDevice::~PhysicalDevice() = default;

PhysicalDevice::operator vk::PhysicalDevice() const
{
  return physical_device_;
}

const vk::PhysicalDeviceProperties& PhysicalDevice::Properties() const
{
  return physical_device_.getProperties();
}

const vk::PhysicalDeviceFeatures& PhysicalDevice::Features() const
{
  return physical_device_.getFeatures();
}
}
}
