#include <twopi/vkl/vkl_stage_buffer.h>

#include <cstring>
#include <algorithm>

namespace twopi
{
namespace vkl
{
StageBuffer::StageBuffer(std::shared_ptr<vkl::Context> context)
  : Object{ context }
{
  constexpr vk::DeviceSize buffer_size = 32 * 1024 * 1024; // 32MB

  const auto device = context->Device();

  vk::BufferCreateInfo buffer_create_info;
  buffer_create_info
    .setSharingMode(vk::SharingMode::eExclusive)
    .setUsage(vk::BufferUsageFlagBits::eTransferSrc)
    .setSize(buffer_size);
  buffer_ = device.createBuffer(buffer_create_info);

  memory_ = context->AllocatePersistentlyMappedMemory(buffer_);
  map_ = device.mapMemory(memory_.device_memory, memory_.offset, memory_.size);
}

StageBuffer::~StageBuffer()
{
  const auto device = Context()->Device();

  device.unmapMemory(memory_.device_memory);
  device.freeMemory(memory_.device_memory);

  device.destroyBuffer(buffer_);
}

StageBuffer::operator void* const () const
{
  return map_;
}

StageBuffer::operator void* ()
{
  return map_;
}
}
}
