#ifndef TWOPI_VK_INTERNAL_VK_DEVICE_HANDLE_H_
#define TWOPI_VK_INTERNAL_VK_DEVICE_HANDLE_H_

#include <twopi/vk/internal/vk_handle.h>

#include <vulkan/vulkan.h>

namespace twopi
{
namespace vk
{
namespace internal
{
class DeviceHandle : public DependentHandle<VkPhysicalDevice, VkDevice>
{
public:
  DeviceHandle() = delete;
  DeviceHandle(VkPhysicalDevice physical_device, VkDevice instance);
  ~DeviceHandle();

  void Destroy() override;

private:
};
}
}
}

#endif // TWOPI_VK_INTERNAL_VK_DEVICE_HANDLE_H_
