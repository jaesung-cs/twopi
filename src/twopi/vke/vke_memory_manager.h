#ifndef TWOPI_VKE_VKE_MEMORY_MANAGER_H_
#define TWOPI_VKE_VKE_MEMORY_MANAGER_H_

#include <memory>

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vke
{
class Context;
class Buffer;
class Image;
class Memory;

class MemoryManager
{
public:
  static constexpr uint32_t alignment = 256;

private:
  // 1GB
  constexpr static uint64_t chunk_size_ = 1024 * 1024 * 1024;

  // Two types of memory:
  // - Device Local
  // - Host Visible | Host Coherent
  inline static const std::array<vk::MemoryPropertyFlags, 2> memory_properties_ = {
    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
    vk::MemoryPropertyFlagBits::eDeviceLocal,
  };

public:
  MemoryManager() = delete;
  MemoryManager(const Context& context);
  ~MemoryManager();

  Memory AllocateHostVisibleMemory(uint64_t size);
  Memory AllocateHostVisibleMemory(vk::Buffer buffer);
  Memory AllocateHostVisibleMemory(vk::Image image);
  Memory AllocateDeviceLocalMemory(uint64_t size);
  Memory AllocateDeviceLocalMemory(vk::Buffer buffer);
  Memory AllocateDeviceLocalMemory(vk::Image buffer);

private:
  Memory AllocateMemory(int memory_index, uint64_t size);

  const Context& context_;
  std::vector<int> memory_type_indices_{ -1, -1 };

  std::array<vk::DeviceMemory, 2> memories_;
  std::array<uint64_t, 2> memory_offsets_{ 0, 0 };
};
}
}

#endif // TWOPI_VKE_VKE_MEMORY_MANAGER_H_
