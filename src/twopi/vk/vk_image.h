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

  ~Image();

  operator vk::Image() const;

private:
  vk::Image image_;
};
}
}

#endif // TWOPI_VK_VK_IMAGE_H_
