#ifndef TWOPI_VKE_VKE_MEMORY_H_
#define TWOPI_VKE_VKE_MEMORY_H_

#include <memory>

namespace twopi
{
namespace vkw
{
class DeviceMemory;
}

namespace vke
{
class Memory
{
public:
  Memory() = delete;

  Memory(vkw::DeviceMemory device_memory, uint64_t offset, uint64_t size);

  ~Memory();

  Memory(const Memory& rhs);
  Memory& operator = (const Memory& rhs);

  Memory(Memory&& rhs) noexcept;
  Memory& operator = (Memory&& rhs) noexcept;

  vkw::DeviceMemory DeviceMemory() const;
  uint64_t Size() const;
  uint64_t Offset() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_VKE_VKE_MEMORY_H_
