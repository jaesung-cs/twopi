#include <twopi/vk/internal/vk_device_handle.h>

#include <iostream>

namespace twopi
{
namespace vk
{
namespace internal
{
DeviceHandle::DeviceHandle(VkPhysicalDevice physical_device, VkDevice instance)
  : DependentHandle(physical_device, instance)
{
}

DeviceHandle::~DeviceHandle()
{
  Destroy();
}

void DeviceHandle::Destroy()
{
  std::cout << "Destroy Device" << std::endl;
  vkDestroyDevice(*this, nullptr);
}
}
}
}
