#include <twopi/vk/vk_device.h>

#include <twopi/core/error.h>
#include <twopi/vk/vk_physical_device.h>
#include <twopi/vk/vk_queue.h>
#include <twopi/vk/vk_surface.h>
#include <twopi/vk/vk_swapchain.h>
#include <twopi/vk/vk_semaphore.h>

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

Device::Creator& Device::Creator::AddPresentQueue(Surface surface)
{
  queue_types_.push_back(QueueType::PRESENT);
  surface_ = surface;
  return *this;
}

Device::Creator& Device::Creator::AddSwapchainExtension()
{
  extensions_.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  return *this;
}

Device Device::Creator::Create()
{
  int graphics_index = -1;
  int present_index = -1;
  const auto queue_family_properties = physical_device_.getQueueFamilyProperties();
  for (int i = 0; i < queue_family_properties.size(); i++)
  {
    if (graphics_index == -1 && queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
      graphics_index = i;

    // TODO: select unique queue family indices
    if ((present_index == -1 || present_index == graphics_index) && physical_device_.getSurfaceSupportKHR(i, surface_))
      present_index = i;
  }

  std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
  std::vector<QueueIndex> queue_indices;
  std::vector<int> queue_indices_in_family(queue_family_properties.size());
  float queue_priority = 1.f;
  for (int i = 0; i < queue_types_.size(); i++)
  {
    const auto& queue_type = queue_types_[i];

    int queue_family_index = -1;
    switch (queue_type)
    {
    case QueueType::GRAPHICS:
      queue_family_index = graphics_index;
      break;

    case QueueType::PRESENT:
      queue_family_index = present_index;
      break;
    }

    vk::DeviceQueueCreateInfo queue_create_info;
    queue_create_info
      .setQueueFamilyIndex(queue_family_index)
      .setQueueCount(1)
      .setQueuePriorities(queue_priority);

    queue_indices.emplace_back(QueueIndex{ queue_family_index, queue_indices_in_family[queue_family_index] });
    queue_indices_in_family[queue_family_index]++;
    queue_create_infos.emplace_back(std::move(queue_create_info));
  }
  create_info_.setQueueCreateInfos(queue_create_infos);

  auto features = physical_device_.getFeatures();
  create_info_.setPEnabledFeatures(&features);

  std::vector<const char*> extension_names;
  std::transform(extensions_.cbegin(), extensions_.cend(), std::back_inserter(extension_names), [](const std::string& extension) {
    return extension.c_str();
    });
  create_info_.setPEnabledExtensionNames(extension_names);

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

Queue Device::Queue(int index) const
{
  const auto& queue_index = queue_indices_[index];
  const auto queue = device_.getQueue(queue_index.family_index, queue_index.queue_index);
  return vkw::Queue{ queue, queue_index.family_index };
}

void Device::Destroy()
{
  device_.destroy();
}

Device::operator vk::Device() const
{
  return device_;
}

std::pair<uint32_t, vk::Result> Device::AcquireNextImage(Swapchain swapchain, Semaphore semaphore)
{
  auto result = device_.acquireNextImageKHR(swapchain, UINT64_MAX, semaphore, nullptr);
  return std::make_pair(result.value, result.result);
}

void Device::WaitIdle()
{
  device_.waitIdle();
}
}
}
