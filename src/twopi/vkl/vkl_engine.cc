#include <twopi/vkl/vkl_engine.h>

#include <iostream>

#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <twopi/window/glfw_window.h>

namespace twopi
{
namespace vkl
{
namespace
{
VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
  VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
  VkDebugUtilsMessageTypeFlagsEXT message_type,
  const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
  void* pUserData)
{
  if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    std::cerr << callback_data->pMessage << std::endl << std::endl;

  return VK_FALSE;
}
}

class Engine::Impl
{
public:
  Impl() = delete;

  Impl(std::shared_ptr<window::Window> window)
  {
    glfw_window_handle_ = std::dynamic_pointer_cast<window::GlfwWindow>(window)->Handle();

    Prepare();
  }

  ~Impl()
  {
    Cleanup();
  }

  void Draw(core::Duration duration)
  {
    std::cout << "Draw " << duration.count() << std::endl;
  }

  void Resize(int width, int height)
  {
    std::cout << "Resized: " << width << ' ' << height << std::endl;
  }

  void UpdateLights(const std::vector<std::shared_ptr<scene::Light>>& lights)
  {
  }

  void UpdateCamera(std::shared_ptr<scene::Camera> camera)
  {
  }

private:
  void Prepare()
  {
    CreateInstance();
  }

  void Cleanup()
  {
    CleanupInstance();
  }

  void CreateInstance()
  {
    // App
    vk::ApplicationInfo app_info;
    app_info
      .setPApplicationName("Twopi")
      .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
      .setPEngineName("Twopi Engine")
      .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
      .setApiVersion(VK_API_VERSION_1_2);

    // Layers
    std::vector<const char*> layers = {
      "VK_LAYER_KHRONOS_validation",
    };

    // Extensions
    std::vector<const char*> extensions = {
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    };

    uint32_t num_glfw_extensions = 0;
    const char** glfw_extensions;
    glfw_extensions = glfwGetRequiredInstanceExtensions(&num_glfw_extensions);
    for (uint32_t i = 0; i < num_glfw_extensions; i++)
      extensions.push_back(glfw_extensions[i]);

    // Messenger
    vk::DebugUtilsMessengerCreateInfoEXT messenger_create_info;
    messenger_create_info
      .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose)
      .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
      .setPfnUserCallback(debug_callback)
      .setPUserData(nullptr);

    // Create instance
    vk::InstanceCreateInfo instance_create_info;
    instance_create_info
      .setPApplicationInfo(&app_info)
      .setPEnabledLayerNames(layers)
      .setPEnabledExtensionNames(extensions)
      .setPNext(&messenger_create_info);

    instance_ = vk::createInstance(instance_create_info);

    // Create messneger
    vk::DynamicLoader dl;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    vk::DispatchLoaderDynamic dld(static_cast<vk::Instance>(instance_), vkGetInstanceProcAddr);
    messenger_ = instance_.createDebugUtilsMessengerEXT(messenger_create_info, nullptr, dld);

    // Create surface
    VkSurfaceKHR surface_handle;
    glfwCreateWindowSurface(instance_, glfw_window_handle_, nullptr, &surface_handle);
    surface_ = surface_handle;
  }

  void CleanupInstance()
  {
    instance_.destroySurfaceKHR(surface_);

    vk::DynamicLoader dl;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    vk::DispatchLoaderDynamic dld(instance_, vkGetInstanceProcAddr);
    instance_.destroyDebugUtilsMessengerEXT(messenger_, nullptr, dld);

    instance_.destroy();
  }

  GLFWwindow* glfw_window_handle_;

  vk::Instance instance_;
  vk::DebugUtilsMessengerEXT messenger_;
  vk::SurfaceKHR surface_;
};

Engine::Engine(std::shared_ptr<window::Window> window)
  : impl_(std::make_unique<Impl>(window))
{
}

Engine::~Engine() = default;

void Engine::Draw(core::Duration duration)
{
  impl_->Draw(duration);
}

void Engine::Resize(int width, int height)
{
  impl_->Resize(width, height);
}

void Engine::UpdateLights(const std::vector<std::shared_ptr<scene::Light>>& lights)
{
  impl_->UpdateLights(lights);
}

void Engine::UpdateCamera(std::shared_ptr<scene::Camera> camera)
{
  impl_->UpdateCamera(camera);
}
}
}
