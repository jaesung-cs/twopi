#include <twopi/vke/vke_buffer.h>

#include <vulkan/vulkan.hpp>

#include <twopi/vke/vke_context.h>
#include <twopi/vke/vke_memory.h>
#include <twopi/vke/vke_memory_manager.h>

namespace twopi
{
namespace vke
{
Buffer::Buffer(std::shared_ptr<vke::Context> context, vk::BufferCreateInfo create_info, MemoryType memory_type)
  : context_(context)
{
  buffer_ = context->Device().createBuffer(create_info);

  switch (memory_type)
  {
  case MemoryType::Host:
    memory_ = std::make_unique<Memory>(context->MemoryManager()->AllocateHostVisibleMemory(buffer_));
    break;
  case MemoryType::Device:
    memory_ = std::make_unique<Memory>(context->MemoryManager()->AllocateDeviceLocalMemory(buffer_));
    break;
  }

  context->Device().bindBufferMemory(buffer_, *memory_, memory_->Size());
}

Buffer::~Buffer()
{
  Context()->Device().destroyBuffer(buffer_);
}

Buffer::operator vk::Buffer() const
{
  return buffer_;
}

void* Buffer::Map()
{
  return Context()->Device().mapMemory(*memory_, memory_->Offset(), memory_->Size());
}

void Buffer::Unmap()
{
  Context()->Device().unmapMemory(*memory_);
}

std::shared_ptr<Context> Buffer::Context() const
{
  return context_.lock();
}
}
}
