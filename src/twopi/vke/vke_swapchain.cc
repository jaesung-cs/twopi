#include <twopi/vke/vke_swapchain.h>

#include <twopi/vke/vke_context.h>
#include <twopi/vke/vke_image.h>

namespace twopi
{
namespace vke
{
Swapchain::Swapchain(std::shared_ptr<vke::Context> context, uint32_t width, uint32_t height)
  : context_(context)
{
  const auto surface = context->Surface();
  const auto device = context->Device();
  const auto physical_device = context->PhysicalDevice();
  const auto capabilities = context->PhysicalDevice().getSurfaceCapabilitiesKHR(surface);

  // Triple buffering
  auto image_count = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
    image_count = capabilities.maxImageCount;

  // Presnet mode
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
    VkExtent2D actual_extent = { width, height };

    actual_extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actual_extent.width));
    actual_extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actual_extent.height));

    extent = actual_extent;
  }

  // Sharing images
  const auto& queue_family_indices = context->QueueFamilyIndices();

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
    .setImageSharingMode(vk::SharingMode::eConcurrent)
    .setQueueFamilyIndices(queue_family_indices);

  swapchain_ = device.createSwapchainKHR(swapchain_create_info);

  format_ = format.format;
  width_ = extent.width;
  height_ = extent.height;

  swapchain_images_ = device.getSwapchainImagesKHR(swapchain_);
  image_count_ = swapchain_images_.size();

  // Swapchain image views
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
    .setFormat(format_)
    .setSubresourceRange(image_subresource_range);

  for (uint32_t i = 0; i < image_count_; i++)
  {
    image_view_create_info
      .setImage(swapchain_images_[i]);

    swapchain_image_views_.emplace_back(device.createImageView(image_view_create_info));
  }

  // Multisample color image
  vk::ImageCreateInfo image_create_info;
  image_create_info
    .setImageType(vk::ImageType::e2D)
    .setMipLevels(1)
    .setArrayLayers(1)
    .setTiling(vk::ImageTiling::eOptimal)
    .setSharingMode(vk::SharingMode::eExclusive)
    .setInitialLayout(vk::ImageLayout::eUndefined)
    .setUsage(vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment)
    .setSamples(vk::SampleCountFlagBits::e4)
    .setExtent({ width_, height_, 1 })
    .setFormat(format_);
  multisample_color_image_ = std::make_unique<Image>(context, image_create_info, Image::MemoryType::Device);

  image_view_create_info
    .setImage(*multisample_color_image_);
  multisample_color_image_view_ = device.createImageView(image_view_create_info);

  // Multisample depth image
  image_create_info
    .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
    .setSamples(vk::SampleCountFlagBits::e4)
    .setFormat(vk::Format::eD24UnormS8Uint);
  multisample_depth_image_ = std::make_unique<Image>(context, image_create_info, Image::MemoryType::Device);

  image_subresource_range
    .setAspectMask(vk::ImageAspectFlagBits::eDepth);

  image_view_create_info
    .setImage(*multisample_depth_image_)
    .setFormat(vk::Format::eD24UnormS8Uint)
    .setSubresourceRange(image_subresource_range);
  multisample_depth_image_view_ = device.createImageView(image_view_create_info);
}

Swapchain::~Swapchain()
{
  const auto device = Context()->Device();

  multisample_color_image_.reset();
  device.destroyImageView(multisample_color_image_view_);

  multisample_depth_image_.reset();
  device.destroyImageView(multisample_depth_image_view_);

  for (auto& swapchain_image_view : swapchain_image_views_)
    device.destroyImageView(swapchain_image_view);
  swapchain_image_views_.clear();

  device.destroySwapchainKHR(swapchain_);
}

vk::Format Swapchain::Format() const
{
  return format_;
}

uint32_t Swapchain::Width() const
{
  return width_;
}

uint32_t Swapchain::Height() const
{
  return height_;
}

uint32_t Swapchain::ImageCount() const
{
  return image_count_;
}

std::shared_ptr<Context> Swapchain::Context() const
{
  return context_.lock();
}
}
}
