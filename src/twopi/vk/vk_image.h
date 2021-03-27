#ifndef TWOPI_VK_VK_IMAGE_H_
#define TWOPI_VK_VK_IMAGE_H_

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Image
{
public:
  Image();
  Image(vk::Image image);
  Image(vk::Image image, vk::Format format);

  ~Image();

  operator vk::Image() const;

  vk::Format Format() const;

private:
  vk::Image image_;
  vk::Format format_;
};
}
}

#endif // TWOPI_VK_VK_IMAGE_H_
