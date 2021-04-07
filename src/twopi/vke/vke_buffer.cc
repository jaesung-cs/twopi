#include <twopi/vke/vke_buffer.h>

#include <vulkan/vulkan.hpp>

#include <twopi/vke/vke_context.h>
#include <twopi/vke/vke_memory.h>
#include <twopi/vke/vke_memory_manager.h>

namespace twopi
{
namespace vke
{
Buffer::Buffer(std::shared_ptr<Context> context, vk::BufferCreateInfo create_info, MemoryType memory_type)
  : context_(context)
{
  buffer_ = context_->Device().createBuffer(create_info);

  switch (memory_type)
  {
  case MemoryType::Host:
    memory_ = std::make_unique<Memory>(context_->MemoryManager()->AllocateHostVisibleMemory(buffer_));
    break;
  case MemoryType::Device:
    memory_ = std::make_unique<Memory>(context_->MemoryManager()->AllocateDeviceLocalMemory(buffer_));
    break;
  }

  context_->Device().bindBufferMemory(buffer_, *memory_, memory_->Size());
}

Buffer::~Buffer() = default;

Buffer::operator vk::Buffer() const
{
  return buffer_;
}

void* Buffer::Map()
{
  return context_->Device().mapMemory(*memory_, memory_->Offset(), memory_->Size());
}

void Buffer::Unmap()
{
  context_->Device().unmapMemory(*memory_);
}
}
}
