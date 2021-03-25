#ifndef TWOPI_VK_INTERNAL_VK_PHYSICAL_DEVICE_HANDLE_H_
#define TWOPI_VK_INTERNAL_VK_PHYSICAL_DEVICE_HANDLE_H_

#include <twopi/vk/internal/vk_handle.h>

#include <vulkan/vulkan.h>

namespace twopi
{
namespace vk
{
namespace internal
{
class PhysicalDeviceHandle : public DependentHandle<VkInstance, VkPhysicalDevice>
{
public:
  PhysicalDeviceHandle() = delete;
  PhysicalDeviceHandle(VkInstance instance, VkPhysicalDevice physical_device);
  ~PhysicalDeviceHandle();

  void Destroy() override;

private:
};
}
}
}

#endif // TWOPI_VK_INTERNAL_VK_PHYSICAL_DEVICE_HANDLE_H_
