#include <twopi/vke/vke_memory.h>

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vke
{
Memory::Memory(vk::DeviceMemory device_memory, uint64_t offset, uint64_t size)
  : device_memory_(device_memory)
  , offset_(offset)
  , size_(size)
{
}

Memory::~Memory() = default;

Memory::operator vk::DeviceMemory() const
{
  return device_memory_;
}

uint64_t Memory::Size() const
{
  return size_;
}

uint64_t Memory::Offset() const
{
  return offset_;
}
}
}
