#include <twopi/vk/vk_image.h>

namespace twopi
{
namespace vkw
{
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

Image::~Image() = default;

Image::operator vk::Image() const
{
  return image_;
}

vk::Format Image::Format() const
{
  return format_;
}
}
}
