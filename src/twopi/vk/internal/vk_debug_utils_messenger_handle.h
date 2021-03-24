#ifndef TWOPI_VK_INTERNAL_VK_DEBUG_UTILS_MESSENGER_HANDLE_H_
#define TWOPI_VK_INTERNAL_VK_DEBUG_UTILS_MESSENGER_HANDLE_H_

#include <twopi/vk/internal/vk_handle.h>

#include <vulkan/vulkan.h>

namespace twopi
{
namespace vk
{
namespace internal
{
class DebugUtilsMessengerHandle : public DependentHandle<VkInstance, VkDebugUtilsMessengerEXT>
{
public:
  DebugUtilsMessengerHandle() = delete;
  DebugUtilsMessengerHandle(VkInstance instance, VkDebugUtilsMessengerEXT debug_utils_messenger);
  ~DebugUtilsMessengerHandle();

  void Destroy() override;

private:
};
}
}
}

#endif // TWOPI_VK_INTERNAL_VK_DEBUG_UTILS_MESSENGER_HANDLE_H_
