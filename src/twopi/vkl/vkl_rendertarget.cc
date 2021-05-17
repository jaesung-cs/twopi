#include <twopi/vkl/vkl_rendertarget.h>

#include <twopi/vkl/vkl_context.h>

namespace twopi
{
namespace vkl
{
Rendertarget::Rendertarget(std::shared_ptr<vkl::Context> context, uint32_t width, uint32_t height, vk::Format format, vk::SampleCountFlagBits samples)
  : Rendertarget{ context, width, height, width, height, format, samples }
{
}

Rendertarget::Rendertarget(std::shared_ptr<vkl::Context> context, uint32_t max_width, uint32_t max_height, uint32_t width, uint32_t height, vk::Format format, vk::SampleCountFlagBits samples)
  : Object(context)
{
  const auto device = context->Device();

  // Allocate image memories with max width/height
  vk::ImageCreateInfo image_create_info;
  image_create_info
    .setImageType(vk::ImageType::e2D)
    .setMipLevels(1)
    .setArrayLayers(1)
    .setTiling(vk::ImageTiling::eOptimal)
    .setInitialLayout(vk::ImageLayout::eUndefined)
    .setSharingMode(vk::SharingMode::eExclusive)
    .setSamples(samples)
    .setExtent({ max_width, max_height, 1 })
    .setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment)
    .setFormat(format);
  auto limit_color_image = device.createImage(image_create_info);

  color_image_memory_ = context->AllocateDeviceMemory(limit_color_image);

  image_create_info
    .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
    .setFormat(vk::Format::eD24UnormS8Uint);
  auto limit_depth_image = device.createImage(image_create_info);

  depth_image_memory_ = context->AllocateDeviceMemory(limit_depth_image);

  device.destroyImage(limit_color_image);
  device.destroyImage(limit_depth_image);

  // Create multisample rendertarget image
  image_create_info
    .setExtent({ width, height, 1 })
    .setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment)
    .setFormat(format);
  color_image_ = device.createImage(image_create_info);

  device.bindImageMemory(color_image_, color_image_memory_.device_memory, color_image_memory_.offset);

  // Multisample color image view
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
    .setFormat(format)
    .setSubresourceRange(image_subresource_range);

  image_view_create_info
    .setImage(color_image_);
  color_image_view_ = device.createImageView(image_view_create_info);

  // Create multisample depth image
  image_create_info
    .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
    .setFormat(vk::Format::eD24UnormS8Uint);
  depth_image_ = device.createImage(image_create_info);

  device.bindImageMemory(depth_image_, depth_image_memory_.device_memory, depth_image_memory_.offset);

  // Multisample depth image view
  image_subresource_range
    .setAspectMask(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil);

  image_view_create_info
    .setSubresourceRange(image_subresource_range)
    .setFormat(vk::Format::eD24UnormS8Uint)
    .setImage(depth_image_);
  depth_image_view_ = device.createImageView(image_view_create_info);
}

Rendertarget::~Rendertarget()
{
  const auto device = Context()->Device();

  device.destroyImage(color_image_);
  device.destroyImageView(color_image_view_);

  device.destroyImage(depth_image_);
  device.destroyImageView(depth_image_view_);
}
}
}
