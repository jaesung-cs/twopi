#include <twopi/vkl/vkl_uniform_buffer.h>

#include <twopi/vkl/vkl_context.h>

namespace twopi
{
namespace vkl
{
UniformBuffer::UniformBuffer(std::shared_ptr<vkl::Context> context)
  : Object{ context }
{
  const auto device = context->Device();
  const auto physical_device = context->PhysicalDevice();

  ubo_alignment_ = physical_device.getProperties().limits.minUniformBufferOffsetAlignment;

  // Uniform buffers
  vk::BufferCreateInfo buffer_create_info;
  buffer_create_info
    .setSharingMode(vk::SharingMode::eExclusive)
    .setUsage(vk::BufferUsageFlagBits::eUniformBuffer)
    .setSize(256 * 1024 * 1024); // 256MB

  buffer_ = device.createBuffer(buffer_create_info);
  memory_ = context->AllocatePersistentlyMappedMemory(buffer_);
  device.bindBufferMemory(buffer_, memory_.device_memory, memory_.offset);
  map_ = static_cast<unsigned char*>(device.mapMemory(memory_.device_memory, memory_.offset, memory_.size));
}

UniformBuffer::~UniformBuffer()
{
  const auto device = Context()->Device();

  device.unmapMemory(memory_.device_memory);
  device.freeMemory(memory_.device_memory);
  device.destroyBuffer(buffer_);
}
}
}
