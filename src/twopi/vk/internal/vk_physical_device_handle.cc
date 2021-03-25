#include <twopi/vk/internal/vk_physical_device_handle.h>

#include <iostream>

namespace twopi
{
namespace vk
{
namespace internal
{
PhysicalDeviceHandle::PhysicalDeviceHandle(VkInstance instance, VkPhysicalDevice physical_device)
  : DependentHandle(instance, physical_device)
{
}

PhysicalDeviceHandle::~PhysicalDeviceHandle()
{
  Destroy();
}

void PhysicalDeviceHandle::Destroy()
{
  std::cout << "Destroy Physical Device (implicitly created from instance)" << std::endl;
}
}
}
}
