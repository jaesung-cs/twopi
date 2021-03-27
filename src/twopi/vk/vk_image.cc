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

Image::~Image() = default;

Image::operator vk::Image() const
{
  return image_;
}
}
}
