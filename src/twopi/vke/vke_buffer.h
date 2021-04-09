#ifndef TWOPI_VKE_VKE_BUFFER_H_
#define TWOPI_VKE_VKE_BUFFER_H_

#include <memory>

namespace twopi
{
namespace vkw
{
class Buffer;
class DeviceMemory;
}

namespace vke
{
class Memory;

class Buffer
{
public:
  Buffer() = delete;
  Buffer(vkw::Buffer&& buffer, vkw::DeviceMemory memory, uint64_t offset, uint64_t size);
  Buffer(vkw::Buffer&& buffer, Memory memory);
  ~Buffer();

  operator vkw::Buffer() const;
  uint64_t Offset() const;

  void* Map();
  void Unmap();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_VKE_VKE_BUFFER_H_
