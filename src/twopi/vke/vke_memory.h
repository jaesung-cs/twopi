#ifndef TWOPI_VKE_VKE_MEMORY_H_
#define TWOPI_VKE_VKE_MEMORY_H_

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vke
{
class Memory
{
public:
  Memory() = delete;

  Memory(vk::DeviceMemory device_memory, uint64_t offset, uint64_t size);

  ~Memory();

  operator vk::DeviceMemory() const;
  uint64_t Size() const;
  uint64_t Offset() const;

private:
  vk::DeviceMemory device_memory_;
  uint64_t offset_;
  uint64_t size_;
};
}
}

#endif // TWOPI_VKE_VKE_MEMORY_H_
