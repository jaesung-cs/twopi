#include <twopi/vkl/vkl_stage_buffer.h>

#include <cstring>
#include <algorithm>

#include <twopi/vkl/vkl_context.h>

namespace twopi
{
namespace vkl
{
StageBuffer::StageBuffer(vkl::Context* context)
  : context_(context)
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
  device.bindBufferMemory(buffer_, memory_.device_memory, memory_.offset);
}

StageBuffer::~StageBuffer()
{
  const auto device = context_->Device();

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
