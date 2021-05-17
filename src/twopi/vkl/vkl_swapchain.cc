#include <twopi/vkl/vkl_swapchain.h>

#include <twopi/core/error.h>

#include <twopi/vkl/vkl_context.h>

namespace twopi
{
namespace vkl
{
Swapchain::Swapchain(std::shared_ptr<vkl::Context> context, uint32_t width, uint32_t height)
  : Object{ context }
{
  const auto physical_device = context->PhysicalDevice();
  const auto device = context->Device();
  const auto surface = context->Surface();

  const auto capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);

  // Triple buffering
  auto image_count = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
    image_count = capabilities.maxImageCount;

  if (image_count != 3)
    throw core::Error("Triple buffering is not supported.");

  vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo;
  const auto present_modes = physical_device.getSurfacePresentModesKHR(surface);
  for (auto available_mode : present_modes)
  {
    if (available_mode == vk::PresentModeKHR::eMailbox)
      present_mode = vk::PresentModeKHR::eMailbox;
  }

  // Format
  const auto available_formats = physical_device.getSurfaceFormatsKHR(surface);
  auto format = available_formats[0];
  for (const auto& available_format : available_formats)
  {
    if (available_format.format == vk::Format::eB8G8R8A8Srgb &&
      available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
      format = available_format;
  }

  // Extent
  vk::Extent2D extent;
  if (capabilities.currentExtent.width != UINT32_MAX)
    extent = capabilities.currentExtent;
  else
  {
    VkExtent2D actual_extent = { width_, height_ };

    actual_extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actual_extent.width));
    actual_extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actual_extent.height));

    extent = actual_extent;
  }

  // Create swapchain
  vk::SwapchainCreateInfoKHR swapchain_create_info;
  swapchain_create_info
    .setSurface(surface)
    .setImageArrayLayers(1)
    .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
    .setPreTransform(capabilities.currentTransform)
    .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
    .setClipped(VK_TRUE)
    .setOldSwapchain(nullptr)
    .setMinImageCount(image_count)
    .setPresentMode(present_mode)
    .setImageFormat(format.format)
    .setImageColorSpace(format.colorSpace)
    .setImageExtent(extent)
    .setImageSharingMode(vk::SharingMode::eExclusive);

  swapchain_ = device.createSwapchainKHR(swapchain_create_info);
  image_format_ = format.format;
  image_count_ = image_count;

  images_ = device.getSwapchainImagesKHR(swapchain_);

  // Create image view for swapchain
  vk::ImageSubresourceRange image_subresource_range;
  image_subresource_range
    .setAspectMask(vk::ImageAspectFlagBits::eColor)
    .setLevelCount(1)
    .setBaseMipLevel(0)
    .setLayerCount(1)
    .setBaseArrayLayer(0);

  vk::ImageViewCreateInfo image_view_create_info;
  image_view_create_info
    .setViewType(vk::ImageViewType::e2D)
    .setComponents(vk::ComponentMapping{})
    .setFormat(image_format_)
    .setSubresourceRange(image_subresource_range);

  image_views_.resize(images_.size());
  for (int i = 0; i < images_.size(); i++)
  {
    image_view_create_info
      .setImage(images_[i]);

    image_views_[i] = device.createImageView(image_view_create_info);
  }
}

Swapchain::~Swapchain()
{
  const auto device = Context()->Device();

  for (auto& image_view : image_views_)
    device.destroyImageView(image_view);
  image_views_.clear();

  device.destroySwapchainKHR(swapchain_);
}
}
}
