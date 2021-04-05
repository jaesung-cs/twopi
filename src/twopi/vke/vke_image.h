#ifndef TWOPI_VKE_VKE_IMAGE_H_
#define TWOPI_VKE_VKE_IMAGE_H_

#include <memory>

namespace twopi
{
namespace vkw
{
class Image;
class DeviceMemory;
}

namespace vke
{
class Memory;

class Image
{
public:
  Image() = delete;
  Image(vkw::Image&& image, vkw::DeviceMemory memory, uint64_t offset, uint64_t size);
  Image(vkw::Image&& image, Memory memory);
  ~Image();

  operator vkw::Image() const;
  uint64_t Offset() const;

  void* Map();
  void Unmap();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_VKE_VKE_IMAGE_H_
