#include <twopi/vk/internal/vk_debug_utils_messenger_handle.h>

#include <iostream>

namespace twopi
{
namespace vk
{
namespace internal
{
namespace
{
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr)
    func(instance, debugMessenger, pAllocator);
}
}

DebugUtilsMessengerHandle::DebugUtilsMessengerHandle(VkInstance instance, VkDebugUtilsMessengerEXT debug_utils_messenger)
  : DependentHandle(instance, debug_utils_messenger)
{
}

DebugUtilsMessengerHandle::~DebugUtilsMessengerHandle()
{
  Destroy();
}

void DebugUtilsMessengerHandle::Destroy()
{
  std::cout << "Destroy DebugUtilsMessenger" << std::endl;
  DestroyDebugUtilsMessengerEXT(Parent(), *this, nullptr);
}
}
}
}
