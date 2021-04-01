#include <twopi/vk/vk_image.h>

#include <twopi/vk/vk_device.h>
#include <twopi/vk/vk_device_memory.h>

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
    .setFormat(format_)
    .setTiling(vk::ImageTiling::eOptimal)
    .setInitialLayout(vk::ImageLayout::eUndefined)
    .setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
    .setSharingMode(vk::SharingMode::eExclusive)
    .setSamples(vk::SampleCountFlagBits::e1);
}

Image::Creator::~Creator() = default;

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

void Image::Bind(DeviceMemory memory)
{
  device_.bindImageMemory(image_, memory, 0);
}
}
}
