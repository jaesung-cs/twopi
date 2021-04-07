#ifndef TWOPI_VKE_VKE_CONTEXT_H_
#define TWOPI_VKE_VKE_CONTEXT_H_

#include <memory>

#include <vulkan/vulkan.hpp>

struct GLFWwindow;

namespace twopi
{
namespace vke
{
class MemoryManager;

class Context
{
private:
  using MemoryManagerType = MemoryManager;

public:
  Context(GLFWwindow* window);
  ~Context();

  vk::PhysicalDevice PhysicalDevice() const;
  vk::Device Device() const;
  std::shared_ptr<MemoryManagerType> MemoryManager() const;

private:
  vk::Instance instance_;
  vk::DebugUtilsMessengerEXT messenger_;

  vk::SurfaceKHR surface_;

  vk::PhysicalDevice physical_device_;
  vk::Device device_;

  vk::Queue graphics_queue_;
  vk::Queue present_queue_;

  std::shared_ptr<MemoryManagerType> memory_manager_;
};
}
}

#endif // TWOPI_VKE_VKE_CONTEXT_H_
