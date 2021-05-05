#include <twopi/vkl/vkl_memory_manager.h>

#include <twopi/vkl/vkl_context.h>
#include <twopi/vkl/vkl_memory.h>

namespace twopi
{
namespace vkl
{
MemoryManager::MemoryManager(Context* context)
  : context_(context)
{
  const auto physical_device = context_->PhysicalDevice();
  const auto device = context_->Device();

  uint32_t device_index = 0;

  // Find memroy type index
  uint64_t device_available_size = 0;
  uint64_t host_available_size = 0;
  const auto memory_properties = physical_device.getMemoryProperties();
  for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
  {
    const auto properties = memory_properties.memoryTypes[i].propertyFlags;
    const auto heap_index = memory_properties.memoryTypes[i].heapIndex;
    const auto heap = memory_properties.memoryHeaps[heap_index];

    if ((properties & vk::MemoryPropertyFlagBits::eDeviceLocal) == vk::MemoryPropertyFlagBits::eDeviceLocal)
    {
      if (heap.size > device_available_size)
      {
        device_index = i;
        device_available_size = heap.size;
      }
    }

    if ((properties & (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent))
      == (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent))
    {
      if (heap.size > host_available_size)
      {
        host_index_ = i;
        host_available_size = heap.size;
      }
    }
  }

  constexpr uint64_t chunk_size = 256 * 1024 * 1024; // 256MB

  vk::MemoryAllocateInfo allocate_info;
  allocate_info
    .setAllocationSize(chunk_size)
    .setMemoryTypeIndex(device_index);
  device_memory_ = device.allocateMemory(allocate_info);

  allocate_info
    .setMemoryTypeIndex(host_index_);
  host_memory_ = device.allocateMemory(allocate_info);
}

MemoryManager::~MemoryManager()
{
  const auto device = context_->Device();

  device.freeMemory(device_memory_);
  device.freeMemory(host_memory_);
}

Memory MemoryManager::AllocateDeviceMemory(vk::Buffer buffer)
{
  const auto device = context_->Device();

  return AllocateDeviceMemory(device.getBufferMemoryRequirements(buffer));
}

Memory MemoryManager::AllocateDeviceMemory(vk::Image image)
{
  const auto device = context_->Device();

  return AllocateDeviceMemory(device.getImageMemoryRequirements(image));
}

Memory MemoryManager::AllocateHostMemory(vk::Buffer buffer)
{
  const auto device = context_->Device();

  return AllocateHostMemory(device.getBufferMemoryRequirements(buffer));
}

Memory MemoryManager::AllocateHostMemory(vk::Image image)
{
  const auto device = context_->Device();

  return AllocateHostMemory(device.getImageMemoryRequirements(image));
}

Memory MemoryManager::AllocatePersistentlyMappedMemory(vk::Image image)
{
  const auto device = context_->Device();

  return AllocatePersistentlyMappedMemory(device.getImageMemoryRequirements(image));
}

Memory MemoryManager::AllocatePersistentlyMappedMemory(vk::Buffer buffer)
{
  const auto device = context_->Device();

  return AllocatePersistentlyMappedMemory(device.getBufferMemoryRequirements(buffer));
}

Memory MemoryManager::AllocateDeviceMemory(const vk::MemoryRequirements& requirements)
{
  Memory memory;
  memory.device_memory = device_memory_;
  memory.offset = (device_memory_offset_ + requirements.alignment - 1ull) & ~(requirements.alignment - 1ull);
  memory.size = requirements.size;
  device_memory_offset_ = memory.offset + memory.size;
  return memory;
}

Memory MemoryManager::AllocateHostMemory(const vk::MemoryRequirements& requirements)
{
  Memory memory;
  memory.device_memory = host_memory_;
  memory.offset = (host_memory_offset_ + requirements.alignment - 1ull) & ~(requirements.alignment - 1ull);
  memory.size = requirements.size;
  host_memory_offset_ = memory.offset + memory.size;
  return memory;
}

Memory MemoryManager::AllocatePersistentlyMappedMemory(const vk::MemoryRequirements& requirements)
{
  const auto device = context_->Device();

  vk::MemoryAllocateInfo allocate_info;
  allocate_info
    .setMemoryTypeIndex(host_index_)
    .setAllocationSize(requirements.size);

  Memory memory;
  memory.device_memory = device.allocateMemory(allocate_info);
  memory.offset = 0;
  memory.size = requirements.size;
  return memory;
}
}
}
