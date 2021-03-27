#include <twopi/vk/vk_physical_device.h>

namespace twopi
{
namespace vkw
{
PhysicalDevice::PhysicalDevice()
{
}

PhysicalDevice::PhysicalDevice(vk::PhysicalDevice physical_device)
  : physical_device_(physical_device)
{
}

PhysicalDevice::~PhysicalDevice() = default;

PhysicalDevice::operator vk::PhysicalDevice() const
{
  return physical_device_;
}

std::vector<vk::ExtensionProperties> PhysicalDevice::Extensions() const
{
  return physical_device_.enumerateDeviceExtensionProperties();
}

vk::PhysicalDeviceProperties PhysicalDevice::Properties() const
{
  return physical_device_.getProperties();
}

vk::PhysicalDeviceProperties2 PhysicalDevice::Properties2() const
{
  return physical_device_.getProperties2();
}

vk::PhysicalDeviceFeatures PhysicalDevice::Features() const
{
  return physical_device_.getFeatures();
}

vk::PhysicalDeviceMemoryProperties PhysicalDevice::MemoryProperties() const
{
  return physical_device_.getMemoryProperties();
}
}
}
