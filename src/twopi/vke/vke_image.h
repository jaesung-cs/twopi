#ifndef TWOPI_VKE_VKE_IMAGE_H_
#define TWOPI_VKE_VKE_IMAGE_H_

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vke
{
class Context;
class Memory;

class Image
{
public:
  enum class MemoryType
  {
    Host,
    Device,
  };

public:
  Image() = delete;
  Image(std::shared_ptr<Context> context, vk::ImageCreateInfo create_info, MemoryType memory_type);
  ~Image();

  operator vk::Image() const;

  void* Map();
  void Unmap();

private:
  std::shared_ptr<Context> context_;

  vk::Image image_;

  std::unique_ptr<Memory> memory_;

  uint32_t width_;
  uint32_t height_;
  vk::Format format_;
  uint32_t mip_levels_;
};
}
}

#endif // TWOPI_VKE_VKE_IMAGE_H_
