#ifndef TWOPI_VK_INTERNAL_VK_INSTANCE_HANDLE_H_
#define TWOPI_VK_INTERNAL_VK_INSTANCE_HANDLE_H_

#include <twopi/vk/internal/vk_handle.h>

#include <vulkan/vulkan.h>

namespace twopi
{
namespace vk
{
namespace internal
{
class InstanceHandle : public Handle<VkInstance>
{
public:
  InstanceHandle() = delete;
  InstanceHandle(VkInstance instance);
  ~InstanceHandle();

  void Destroy() override;

private:
};
}
}
}

#endif // TWOPI_VK_INTERNAL_VK_INSTANCE_HANDLE_H_
