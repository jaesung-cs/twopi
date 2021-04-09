#include <twopi/vkw/vkw_image.h>

#include <twopi/vkw/vkw_device.h>
#include <twopi/vkw/vkw_device_memory.h>

namespace twopi
{
namespace vkw
{
//
// Creator
// 
Image::Creator::Creator(Device device)
  : device_(device)
{
  format_ = vk::Format::eR8G8B8A8Srgb;

  create_info_
    .setImageType(vk::ImageType::e2D)
    .setMipLevels(1)
    .setArrayLayers(1)
    .setTiling(vk::ImageTiling::eOptimal)
    .setInitialLayout(vk::ImageLayout::eUndefined)
    .setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
    .setSharingMode(vk::SharingMode::eExclusive)
    .setSamples(vk::SampleCountFlagBits::e1);
}

Image::Creator::~Creator() = default;

Image::Creator& Image::Creator::SetTransferSrc()
{
  create_info_
    .setUsage(vk::ImageUsageFlagBits::eTransferSrc | create_info_.usage);
  return *this;
}

Image::Creator& Image::Creator::SetMipLevels(int mip_levels)
{
  create_info_.setMipLevels(mip_levels);
  return *this;
}

Image::Creator& Image::Creator::SetMultisample4()
{
  create_info_.setSamples(vk::SampleCountFlagBits::e4);
  return *this;
}

Image::Creator& Image::Creator::SetTransientColorAttachment()
{
  create_info_
    .setUsage(vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment);

  return *this;
}

Image::Creator& Image::Creator::SetDepthStencilImage()
{
  format_ = vk::Format::eD24UnormS8Uint;

  create_info_
    .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment);

  return *this;
}

Image::Creator& Image::Creator::SetFormat(vk::Format format)
{
  format_ = format;
  return *this;
}

Image::Creator& Image::Creator::SetSize(int width, int height)
{
  width_ = width;
  height_ = height;

  create_info_
    .setExtent(vk::Extent3D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 });

  return *this;
}

Image Image::Creator::Create()
{
  create_info_.setFormat(format_);

  const auto handle = device_.createImage(create_info_);
  auto image = Image{ device_, handle , format_ };
  image.width_ = width_;
  image.height_ = height_;
  return image;
}

//
// Image
//
Image::Image()
{
}

Image::Image(vk::Image image)
  : image_(image)
{
}

Image::Image(vk::Image image, vk::Format format)
  : image_(image), format_(format)
{
}

Image::Image(vk::Device device, vk::Image image)
  : device_(device), image_(image)
{
}

Image::Image(vk::Device device, vk::Image image, vk::Format format)
  : device_(device), image_(image), format_(format)
{
}

Image::~Image() = default;

void Image::Destroy()
{
  if (device_)
    device_.destroyImage(image_);
}

Image::operator vk::Image() const
{
  return image_;
}

vk::Format Image::Format() const
{
  return format_;
}

vk::DeviceSize Image::RequiredMemorySize() const
{
  return device_.getImageMemoryRequirements(image_).size;
}

void Image::Bind(DeviceMemory memory, uint64_t offset)
{
  device_.bindImageMemory(image_, memory, offset);
}
}
}
