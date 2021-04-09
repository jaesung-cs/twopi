#ifndef TWOPI_VKE_VKE_MEMORY_MANAGER_H_
#define TWOPI_VKE_VKE_MEMORY_MANAGER_H_

#include <memory>

namespace twopi
{
namespace vkw
{
class Device;
class DeviceDevice;
}

namespace vke
{
class Memory;

class MemoryManager
{
public:
  static constexpr uint32_t alignment = 1024;

public:
  MemoryManager() = delete;
  explicit MemoryManager(vkw::Device device);
  ~MemoryManager();

  Memory AllocatePersistenlyMappedMemory(uint64_t size);
  Memory AllocateHostVisibleMemory(uint64_t size);
  Memory AllocateDeviceLocalMemory(uint64_t size);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_VKE_VKE_MEMORY_MANAGER_H_
