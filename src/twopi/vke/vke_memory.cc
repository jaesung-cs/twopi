#include <twopi/vke/vke_memory.h>

#include <twopi/vkw/vkw_device_memory.h>

namespace twopi
{
namespace vke
{
class Memory::Impl
{
public:
  Impl(vkw::DeviceMemory device_memory, uint64_t offset, uint64_t size)
    : device_memory_(device_memory), offset_(offset), size_(size)
  {
  }

  ~Impl() = default;

  vkw::DeviceMemory DeviceMemory() const { return device_memory_; }
  uint64_t Size() const { return size_; }
  uint64_t Offset() const { return offset_; }

private:
  vkw::DeviceMemory device_memory_;
  uint64_t offset_ = 0;
  uint64_t size_ = 0;
};

Memory::Memory(vkw::DeviceMemory device_memory, uint64_t offset, uint64_t size)
  : impl_(std::make_unique<Impl>(device_memory, offset, size))
{
}

Memory::~Memory() = default;

Memory::Memory(const Memory& rhs)
  : impl_(std::make_unique<Impl>(*rhs.impl_))
{
}

Memory& Memory::operator = (const Memory& rhs)
{
  impl_ = std::make_unique<Impl>(*rhs.impl_);
  return *this;
}

Memory::Memory(Memory&& rhs) noexcept = default;
Memory& Memory::operator = (Memory&& rhs) noexcept = default;

vkw::DeviceMemory Memory::DeviceMemory() const
{
  return impl_->DeviceMemory();
}

uint64_t Memory::Size() const
{
  return impl_->Size();
}

uint64_t Memory::Offset() const
{
  return impl_->Offset();
}
}
}
