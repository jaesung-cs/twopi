#include <twopi/vkw/vkw_device_memory.h>

#include <twopi/vkw/vkw_image.h>
#include <twopi/vkw/vkw_device.h>
#include <twopi/vkw/vkw_buffer.h>
#include <twopi/vkw/vkw_physical_device.h>

namespace twopi
{
namespace vkw
{
//
// Allocator
//
DeviceMemory::Allocator::Allocator(const Device& device)
  : device_(device)
{
}

DeviceMemory::Allocator::~Allocator() = default;

DeviceMemory::Allocator& DeviceMemory::Allocator::SetSize(uint64_t size)
{
  allocate_info_
    .setAllocationSize(size);
  size_ = size;

  return *this;
}

DeviceMemory::Allocator& DeviceMemory::Allocator::SetMemoryTypeIndex(uint32_t memory_type_index)
{
  allocate_info_.setMemoryTypeIndex(memory_type_index);
  return *this;
}

DeviceMemory::Allocator& DeviceMemory::Allocator::SetDeviceLocalMemory(Image image, PhysicalDevice physical_device)
{
  constexpr auto required_memory_properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
  SetMemory(image, physical_device, required_memory_properties);
  return *this;
}

DeviceMemory::Allocator& DeviceMemory::Allocator::SetDeviceLocalMemory(Buffer buffer, PhysicalDevice physical_device)
{
  constexpr auto required_memory_properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
  SetMemory(buffer, physical_device, required_memory_properties);
  return *this;
}

DeviceMemory::Allocator& DeviceMemory::Allocator::SetDeviceLocalMemory(PhysicalDevice physical_device)
{
  constexpr auto required_memory_properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
  SetMemory(physical_device, required_memory_properties);
  return *this;
}

DeviceMemory::Allocator& DeviceMemory::Allocator::SetHostVisibleCoherentMemory(Buffer buffer, PhysicalDevice physical_device)
{
  constexpr auto required_memory_properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
  SetMemory(buffer, physical_device, required_memory_properties);
  return *this;
}

DeviceMemory::Allocator& DeviceMemory::Allocator::SetHostVisibleCoherentMemory(PhysicalDevice physical_device)
{
  constexpr auto required_memory_properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
  SetMemory(physical_device, required_memory_properties);
  return *this;
}

void DeviceMemory::Allocator::SetMemory(Image image, PhysicalDevice physical_device, vk::MemoryPropertyFlags required_memory_properties)
{
  const auto memory_requirements = device_.MemoryRequirements(image);
  SetMemory(memory_requirements, physical_device, required_memory_properties);
}

void DeviceMemory::Allocator::SetMemory(Buffer buffer, PhysicalDevice physical_device, vk::MemoryPropertyFlags required_memory_properties)
{
  const auto memory_requirements = device_.MemoryRequirements(buffer);
  SetMemory(memory_requirements, physical_device, required_memory_properties);
}

void DeviceMemory::Allocator::SetMemory(vk::MemoryRequirements memory_requirements, PhysicalDevice physical_device, vk::MemoryPropertyFlags required_memory_properties)
{
  const auto memory_properties = physical_device.MemoryProperties();

  int memory_type_index = -1;
  for (int i = 0; i < memory_properties.memoryTypeCount; i++)
  {
    if ((memory_requirements.memoryTypeBits & (1 << i)) &&
      (memory_properties.memoryTypes[i].propertyFlags & required_memory_properties) == required_memory_properties)
    {
      memory_type_index = i;
      break;
    }
  }

  allocate_info_
    .setAllocationSize(memory_requirements.size)
    .setMemoryTypeIndex(memory_type_index);

  size_ = memory_requirements.size;
}

void DeviceMemory::Allocator::SetMemory(PhysicalDevice physical_device, vk::MemoryPropertyFlags required_memory_properties)
{
  const auto memory_properties = physical_device.MemoryProperties();

  int memory_type_index = -1;
  for (int i = 0; i < memory_properties.memoryTypeCount; i++)
  {
    if ((memory_properties.memoryTypes[i].propertyFlags & required_memory_properties) == required_memory_properties)
    {
      memory_type_index = i;
      break;
    }
  }

  allocate_info_
    .setMemoryTypeIndex(memory_type_index);
}

DeviceMemory DeviceMemory::Allocator::Allocate()
{
  const auto handle = static_cast<vk::Device>(device_).allocateMemory(allocate_info_);
  auto device_memory = DeviceMemory{ device_, handle };
  device_memory.size_ = size_;
  return device_memory;
}

//
// DeviceMemory
//
DeviceMemory::DeviceMemory()
{
}

DeviceMemory::DeviceMemory(vk::Device device, vk::DeviceMemory device_memory)
  : device_(device), device_memory_(device_memory)
{
}

DeviceMemory::~DeviceMemory() = default;

void DeviceMemory::Free()
{
  device_.freeMemory(device_memory_);
}

DeviceMemory::operator vk::DeviceMemory() const
{
  return device_memory_;
}

void* DeviceMemory::Map(uint64_t offset)
{
  return device_.mapMemory(device_memory_, offset, size_ - offset);
}

void DeviceMemory::Unmap()
{
  device_.unmapMemory(device_memory_);
}
}
}
