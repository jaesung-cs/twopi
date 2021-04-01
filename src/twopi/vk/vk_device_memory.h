#ifndef TWOPI_VK_VK_DEVICE_MEMORY_H_
#define TWOPI_VK_VK_DEVICE_MEMORY_H_

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Image;
class Buffer;
class Device;
class PhysicalDevice;

class DeviceMemory
{
public:
  class Allocator
  {
  public:
    Allocator() = delete;
    explicit Allocator(const Device& device);
    ~Allocator();

    Allocator& SetDeviceLocalMemory(Image image, PhysicalDevice physical_device);
    Allocator& SetDeviceLocalMemory(Buffer buffer, PhysicalDevice physical_device);
    Allocator& SetHostVisibleCoherentMemory(Buffer buffer, PhysicalDevice physical_device);

    DeviceMemory Allocate();

  private:
    void SetMemory(Image image, PhysicalDevice physical_device, vk::MemoryPropertyFlags required_memory_properties);
    void SetMemory(Buffer buffer, PhysicalDevice physical_device, vk::MemoryPropertyFlags required_memory_properties);
    void SetMemory(vk::MemoryRequirements memory_requirements, PhysicalDevice physical_device, vk::MemoryPropertyFlags required_memory_properties);

    const Device& device_;

    uint64_t size_ = 0;
    vk::MemoryAllocateInfo allocate_info_{};
  };

public:
  DeviceMemory();
  DeviceMemory(vk::Device device, vk::DeviceMemory device_memory);

  ~DeviceMemory();

  void Free();

  operator vk::DeviceMemory() const;

  void* Map();
  void Unmap();

private:
  vk::Device device_;
  vk::DeviceMemory device_memory_;

  uint64_t size_ = 0;
};
}
}

#endif // TWOPI_VK_VK_DEVICE_MEMORY_H_
