#include <twopi/vke/vke_buffer.h>

#include <twopi/vkw/vkw_buffer.h>
#include <twopi/vkw/vkw_device_memory.h>
#include <twopi/vke/vke_memory.h>

namespace twopi
{
namespace vke
{
class Buffer::Impl
{
public:
  Impl() = delete;

  Impl(vkw::Buffer&& buffer, vkw::DeviceMemory memory, uint64_t offset, uint64_t size)
    : Impl(std::move(buffer), Memory(memory, offset, size)) { }

  Impl(vkw::Buffer&& buffer, Memory memory)
    : buffer_(std::move(buffer)), memory_(memory)
  {
    buffer_.Bind(memory_.DeviceMemory(), memory_.Offset());
  }

  ~Impl()
  {
    buffer_.Destroy();
  }

  operator vkw::Buffer() const
  {
    return buffer_;
  }

  uint64_t Offset() const
  {
    return memory_.Offset();
  }

  void* Map()
  {
    return memory_.DeviceMemory().Map(memory_.Offset(), memory_.Size());
  }

  void Unmap()
  {
    memory_.DeviceMemory().Unmap();
  }

private:
  vkw::Buffer buffer_;
  Memory memory_;
};

Buffer::Buffer(vkw::Buffer&& buffer, vkw::DeviceMemory memory, uint64_t offset, uint64_t size)
  : impl_(std::make_unique<Impl>(std::move(buffer), memory, offset, size))
{
}

Buffer::Buffer(vkw::Buffer&& buffer, Memory memory)
  : impl_(std::make_unique<Impl>(std::move(buffer), memory))
{
}

Buffer::~Buffer() = default;

Buffer::operator vkw::Buffer() const
{
  return impl_->operator vkw::Buffer();
}

uint64_t Buffer::Offset() const
{
  return impl_->Offset();
}

void* Buffer::Map()
{
  return impl_->Map();
}

void Buffer::Unmap()
{
  impl_->Unmap();
}
}
}
