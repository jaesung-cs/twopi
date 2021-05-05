#ifndef TWOPI_VKL_VKL_MEMORY_MANAGER_H_
#define TWOPI_VKL_VKL_MEMORY_MANAGER_H_

#include <mutex>

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkl
{
struct Memory;
class Context;

class MemoryManager
{
public:
  MemoryManager() = delete;
  explicit MemoryManager(Context* context);
  ~MemoryManager();

  Memory AllocateDeviceMemory(vk::Buffer buffer);
  Memory AllocateDeviceMemory(vk::Image image);

  Memory AllocateHostMemory(vk::Buffer buffer);
  Memory AllocateHostMemory(vk::Image image);

  Memory AllocatePersistentlyMappedMemory(vk::Buffer buffer);
  Memory AllocatePersistentlyMappedMemory(vk::Image image);

private:
  const Context* context_;

  Memory AllocateDeviceMemory(const vk::MemoryRequirements& requirements);
  Memory AllocateHostMemory(const vk::MemoryRequirements& requirements);

  Memory AllocatePersistentlyMappedMemory(const vk::MemoryRequirements& requirements);

  std::mutex device_allocate_mutex_;
  vk::DeviceMemory device_memory_;
  vk::DeviceSize device_memory_offset_ = 0;

  uint32_t host_index_ = 0;
  std::mutex host_allocate_mutex_;
  vk::DeviceMemory host_memory_;
  vk::DeviceSize host_memory_offset_ = 0;
};
}
}

#endif // TWOPI_VKL_VKL_MEMORY_MANAGER_H_
