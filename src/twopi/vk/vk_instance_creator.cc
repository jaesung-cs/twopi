#include <twopi/vk/vk_instance_creator.h>

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <twopi/core/error.h>
#include <twopi/vk/vk_instance.h>

namespace twopi
{
namespace vk
{
class InstanceCreator::Impl
{
public:
  Impl()
  {
    app_info_.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info_.pApplicationName = "Twopi";
    app_info_.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info_.pEngineName = "Twopi Engine";
    app_info_.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info_.apiVersion = VK_API_VERSION_1_0;

    create_info_.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info_.pApplicationInfo = &app_info_;
    create_info_.enabledLayerCount = 0;
  }

  ~Impl()
  {
  }

  void AddGlfwRequiredExtensions()
  {
    uint32_t num_glfw_extensions = 0;
    const char** glfw_extensions;

    glfw_extensions = glfwGetRequiredInstanceExtensions(&num_glfw_extensions);

    create_info_.enabledExtensionCount = num_glfw_extensions;
    create_info_.ppEnabledExtensionNames = glfw_extensions;
  }

  Instance Create()
  {
    VkInstance instance;
    const VkResult result = vkCreateInstance(&create_info_, nullptr, &instance);

    if (result != VK_SUCCESS)
      throw core::Error("Failed to create vk instance");

    return Instance{ instance };
  }

private:
  VkApplicationInfo app_info_{};
  VkInstanceCreateInfo create_info_{};
};

InstanceCreator::InstanceCreator()
{
  impl_ = std::make_unique<Impl>();
}

InstanceCreator::~InstanceCreator() = default;

Instance InstanceCreator::Create()
{
  return impl_->Create();
}

InstanceCreator& InstanceCreator::AddGlfwRequiredExtensions()
{
  impl_->AddGlfwRequiredExtensions();
  return *this;
}
}
}
