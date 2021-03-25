#include <twopi/vk/vk_device.h>

#include <twopi/core/error.h>
#include <twopi/vk/vk_physical_device.h>

namespace twopi
{
namespace vkw
{
//
// Creator
//
Device::Creator::Creator(PhysicalDevice physical_device)
{
  // TODO: queue family
  queue_create_info_.queueFamilyIndex = 0;
  queue_create_info_.queueCount = 1;
  queue_create_info_.pQueuePriorities = &queue_priority_;

  create_info_.pQueueCreateInfos = &queue_create_info_;
  create_info_.queueCreateInfoCount = 1;

  create_info_.pEnabledFeatures = &physical_device.Features();

  create_info_.enabledExtensionCount = 0;
  create_info_.enabledLayerCount = 0;
}

Device::Creator::~Creator() = default;

Device Device::Creator::Create()
{
  // TODO
  return Device{};
}

//
// Device
//
Device::Device()
{
}

Device::Device(vk::Device device)
  : device_(device)
{
}

Device::~Device() = default;

void Device::Destroy()
{
  device_.destroy();
}

Device::operator vk::Device() const
{
  return device_;
}
}
}
