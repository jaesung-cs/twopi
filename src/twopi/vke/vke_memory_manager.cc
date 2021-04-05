#include <twopi/vke/vke_memory_manager.h>

#include <array>

#include <twopi/core/error.h>
#include <twopi/vkw/vkw_device.h>
#include <twopi/vkw/vkw_physical_device.h>
#include <twopi/vkw/vkw_device_memory.h>
#include <twopi/vke/vke_memory.h>

namespace twopi
{
namespace vke
{
class MemoryManager::Impl
{
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
  Impl() = delete;

  Impl(vkw::Device device)
    : device_(device)
  {
    const auto physical_device = device_.PhysicalDevice();
    const auto device_memory_property = physical_device.getMemoryProperties();

    for (int i = 0; i < device_memory_property.memoryTypeCount; i++)
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
    memories_[0] = vkw::DeviceMemory::Allocator{ device_ }
      .SetHostVisibleCoherentMemory(vkw::PhysicalDevice(physical_device))
      .SetSize(chunk_size_)
      .Allocate();

    memories_[1] = vkw::DeviceMemory::Allocator{ device_ }
      .SetDeviceLocalMemory(vkw::PhysicalDevice(physical_device))
      .SetSize(chunk_size_)
      .Allocate();
  }

  ~Impl()
  {
    for (int i = 0; i < 2; i++)
      memories_[i].Free();
  }

  Memory AllocateHostVisibleMemory(uint64_t size)
  {
    return AllocateMemory(0, size);
  }

  Memory AllocateDeviceLocalMemory(uint64_t size)
  {
    return AllocateMemory(1, size);
  }

private:
  Memory AllocateMemory(int memory_index, uint64_t size)
  {
    const auto offset = memory_offsets_[memory_index];
    memory_offsets_[memory_index] += (size + alignment - 1) / alignment * alignment;
    return Memory{ memories_[memory_index], offset, size };
  }

  vkw::Device device_;
  std::vector<int> memory_type_indices_{ -1, -1 };

  std::array<vkw::DeviceMemory, 2> memories_;
  std::array<uint64_t, 2> memory_offsets_{ 0, 0 };
};

MemoryManager::MemoryManager(vkw::Device device)
  : impl_(std::make_unique<Impl>(device))
{
}

MemoryManager::~MemoryManager() = default;

Memory MemoryManager::AllocateHostVisibleMemory(uint64_t size)
{
  return impl_->AllocateHostVisibleMemory(size);
}

Memory MemoryManager::AllocateDeviceLocalMemory(uint64_t size)
{
  return impl_->AllocateDeviceLocalMemory(size);
}
}
}
