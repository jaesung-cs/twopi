#ifndef TWOPI_VKE_VKE_BUFFER_H_
#define TWOPI_VKE_VKE_BUFFER_H_

#include <memory>

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vke
{
class Context;
class Memory;

class Buffer
{
public:
  enum class MemoryType
  {
    Host,
    Device,
  };

public:
  Buffer() = delete;
  Buffer(std::shared_ptr<Context> context, vk::BufferCreateInfo create_info, MemoryType memory_type);
  ~Buffer();

  operator vk::Buffer() const;

  void* Map();
  void Unmap();

private:
  std::shared_ptr<Context> context_;

  vk::Buffer buffer_;

  std::unique_ptr<Memory> memory_;
};
}
}

#endif // TWOPI_VKE_VKE_BUFFER_H_
