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
  : physical_device_(physical_device)
{
  // TODO: queue family
  queue_create_info_
    .setQueueFamilyIndex(0)
    .setQueueCount(1)
    .setQueuePriorities(queue_priority_);

  create_info_
    .setQueueCreateInfos(queue_create_info_)
    .setPEnabledExtensionNames({})
    .setPEnabledLayerNames({});
}

Device::Creator::~Creator() = default;

Device Device::Creator::Create()
{
  auto features = physical_device_.getFeatures();
  create_info_.setPEnabledFeatures(&features);

  const auto handle = physical_device_.createDevice(create_info_, nullptr);
  return Device{ handle };
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
