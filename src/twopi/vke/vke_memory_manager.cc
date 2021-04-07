#include <twopi/vke/vke_memory_manager.h>

#include <array>

#include <vulkan/vulkan.hpp>

#include <twopi/core/error.h>
#include <twopi/vke/vke_memory.h>
#include <twopi/vke/vke_context.h>

namespace twopi
{
namespace vke
{
MemoryManager::MemoryManager(const Context& context)
  : context_(context)
{
  auto device = context_.Device();
  auto physical_device = context_.PhysicalDevice();

  const auto device_memory_property = physical_device.getMemoryProperties();

  for (uint32_t i = 0; i < device_memory_property.memoryTypeCount; i++)
  {
    const auto heap_index = device_memory_property.memoryTypes[i].heapIndex;
    auto property = device_memory_property.memoryTypes[i].propertyFlags;

    for (int j = 0; j < 2; j++)
    {
      if ((property & memory_properties_[j]) == memory_properties_[j])
      {
        if (memory_type_indices_[j] == -1 ||
          device_memory_property.memoryHeaps[heap_index].size > device_memory_property.memoryHeaps[device_memory_property.memoryTypes[memory_type_indices_[j]].heapIndex].size)
          memory_type_indices_[j] = i;
      }
    }
  }

  // Pre-allocate one chunk of memory for each memory type
  vk::MemoryAllocateInfo allocate_info;
  allocate_info
    .setMemoryTypeIndex(memory_type_indices_[0])
    .setAllocationSize(chunk_size_);
  memories_[0] = device.allocateMemory(allocate_info);

  allocate_info
    .setMemoryTypeIndex(memory_type_indices_[1]);
  memories_[1] = device.allocateMemory(allocate_info);
}

MemoryManager::~MemoryManager()
{
  for (int i = 0; i < 2; i++)
    context_.Device().freeMemory(memories_[i]);
}

Memory MemoryManager::AllocateHostVisibleMemory(uint64_t size)
{
  return AllocateMemory(0, size);
}

Memory MemoryManager::AllocateHostVisibleMemory(vk::Buffer buffer)
{
  return AllocateMemory(0, context_.Device().getBufferMemoryRequirements(buffer).size);
}

Memory MemoryManager::AllocateHostVisibleMemory(vk::Image image)
{
  return AllocateMemory(0, context_.Device().getImageMemoryRequirements(image).size);
}

Memory MemoryManager::AllocateDeviceLocalMemory(uint64_t size)
{
  return AllocateMemory(1, size);
}

Memory MemoryManager::AllocateDeviceLocalMemory(vk::Buffer buffer)
{
  return AllocateMemory(1, context_.Device().getBufferMemoryRequirements(buffer).size);
}

Memory MemoryManager::AllocateDeviceLocalMemory(vk::Image image)
{
  return AllocateMemory(1, context_.Device().getImageMemoryRequirements(image).size);
}

Memory MemoryManager::AllocateMemory(int memory_index, uint64_t size)
{
  const auto offset = memory_offsets_[memory_index];
  memory_offsets_[memory_index] += (size + alignment - 1) / alignment * alignment;
  return Memory{ memories_[memory_index], offset, size };
}
}
}
