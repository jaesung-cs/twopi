#include <twopi/vk/vk_device.h>

#include <twopi/core/error.h>
#include <twopi/vk/vk_physical_device.h>
#include <twopi/vk/vk_queue.h>

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
  create_info_
    .setPEnabledExtensionNames({})
    .setPEnabledLayerNames({});
}

Device::Creator::~Creator() = default;

Device::Creator& Device::Creator::AddGraphicsQueue()
{
  queue_types_.push_back(QueueType::GRAPHICS);
  return *this;
}

Device Device::Creator::Create()
{
  int graphics_index = -1;
  const auto queue_family_properties = physical_device_.getQueueFamilyProperties();
  for (int i = 0; i < queue_family_properties.size(); i++)
  {
    if (queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
    {
      graphics_index = i;
    }
  }

  std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
  std::vector<QueueIndex> queue_indices;
  std::vector<int> queue_indices_in_family(queue_family_properties.size());
  float queue_priority = 1.f;
  for (int i = 0; i < queue_types_.size(); i++)
  {
    const auto& queue_type = queue_types_[i];
    if (queue_type == QueueType::GRAPHICS)
    {
      vk::DeviceQueueCreateInfo queue_create_info;
      queue_create_info
        .setQueueFamilyIndex(graphics_index)
        .setQueueCount(1)
        .setQueuePriorities(queue_priority);

      queue_indices.emplace_back(QueueIndex{ graphics_index, queue_indices_in_family[graphics_index] });
      queue_indices_in_family[graphics_index]++;

      queue_create_infos.emplace_back(std::move(queue_create_info));
    }
  }
  create_info_.setQueueCreateInfos(queue_create_infos);

  auto features = physical_device_.getFeatures();
  create_info_.setPEnabledFeatures(&features);

  const auto handle = physical_device_.createDevice(create_info_, nullptr);
  auto device = Device{ handle };
  device.queue_indices_ = std::move(queue_indices);
  return device;
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

Queue Device::Queue(int index)
{
  const auto& queue_index = queue_indices_[index];
  const auto queue = device_.getQueue(queue_index.family_index, queue_index.queue_index);
  return vkw::Queue{ queue };
}

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
