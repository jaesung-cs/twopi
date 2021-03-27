#include <twopi/vk/vk_swapchain.h>

#include <set>

#include <twopi/vk/vk_physical_device.h>
#include <twopi/vk/vk_device.h>
#include <twopi/vk/vk_queue.h>
#include <twopi/vk/vk_surface.h>
#include <twopi/vk/vk_image.h>

namespace twopi
{
namespace vkw
{
//
// Creator
//
Swapchain::Creator::Creator(PhysicalDevice physical_device, Device device, Surface surface)
  : physical_device_(physical_device), device_(device), surface_(surface)
{
  create_info_
    .setSurface(surface);

  capabilities_ = physical_device_.getSurfaceCapabilitiesKHR(surface);

  // Default options
  create_info_
    .setImageArrayLayers(1)
    .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
    .setPreTransform(capabilities_.currentTransform)
    .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
    .setClipped(VK_TRUE)
    .setOldSwapchain(nullptr);
}

Swapchain::Creator::~Creator() = default;

Swapchain::Creator& Swapchain::Creator::SetTripleBuffering()
{
  auto image_count = capabilities_.minImageCount + 1;
  vk::PresentModeKHR present_mode = vk::PresentModeKHR::eMailbox;

  create_info_
    .setMinImageCount(image_count)
    .setPresentMode(present_mode);

  return *this;
}

Swapchain::Creator& Swapchain::Creator::SetDefaultFormat()
{
  const auto available_formats = physical_device_.getSurfaceFormatsKHR(surface_);
  auto format = available_formats[0];
  for (const auto& available_format : available_formats)
  {
    if (available_format.format == vk::Format::eB8G8R8A8Srgb && available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
      format = available_format;
  }

  create_info_
    .setImageFormat(format.format)
    .setImageColorSpace(format.colorSpace);

  return *this;
}

Swapchain::Creator& Swapchain::Creator::SetExtent(int width, int height)
{
  if (capabilities_.currentExtent.width != UINT32_MAX)
    create_info_.setImageExtent(capabilities_.currentExtent);
  else
  {
    VkExtent2D actual_extent = {
      static_cast<uint32_t>(width),
      static_cast<uint32_t>(height)
    };

    actual_extent.width = std::max(capabilities_.minImageExtent.width, std::min(capabilities_.maxImageExtent.width, actual_extent.width));
    actual_extent.height = std::max(capabilities_.minImageExtent.height, std::min(capabilities_.maxImageExtent.height, actual_extent.height));

    create_info_.setImageExtent(actual_extent);
  }

  return *this;
}

Swapchain::Creator& Swapchain::Creator::SetQueues(std::initializer_list<Queue> queues)
{
  std::set<uint32_t> queue_family_indices;
  for (const auto& queue : queues)
    queue_family_indices.insert(queue.QueueFamilyIndex());

  if (queue_family_indices.size() > 1)
  {
    create_info_
      .setImageSharingMode(vk::SharingMode::eConcurrent)
      .setQueueFamilyIndices(std::vector<uint32_t>(queue_family_indices.begin(), queue_family_indices.end()));
  }
  else
  {
    create_info_
      .setImageSharingMode(vk::SharingMode::eExclusive)
      .setQueueFamilyIndices({});
  }

  return *this;
}

Swapchain Swapchain::Creator::Create()
{
  auto handle = device_.createSwapchainKHR(create_info_);
  return Swapchain{ device_, handle };
}

//
// Swapchain
//
Swapchain::Swapchain()
{
}

Swapchain::Swapchain(vk::Device device, vk::SwapchainKHR swapchain)
  : device_(device), swapchain_(swapchain)
{
}

Swapchain::~Swapchain() = default;

void Swapchain::Destroy()
{
  device_.destroySwapchainKHR(swapchain_);
}

Swapchain::operator vk::SwapchainKHR() const
{
  return swapchain_;
}

std::vector<Image> Swapchain::Images() const
{
  const auto image_handles = device_.getSwapchainImagesKHR(swapchain_);
  std::vector<Image> images;
  for (const auto& image_handle : image_handles)
    images.emplace_back(image_handle);

  return images;
}
}
}
