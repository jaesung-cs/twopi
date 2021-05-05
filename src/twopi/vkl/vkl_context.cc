#include <twopi/vkl/vkl_context.h>

#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

Context::Context(GLFWwindow* glfw_window)
{
  CreateInstance(glfw_window);
  CreateDevice();
}

Context::~Context()
{
  DestroyDevice();
  DestroyInstance();
}

void Context::CreateInstance(GLFWwindow* glfw_window)
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
  glfwCreateWindowSurface(instance_, glfw_window, nullptr, &surface_handle);
  surface_ = surface_handle;
}

void Context::DestroyInstance()
{
  instance_.destroySurfaceKHR(surface_);

  vk::DynamicLoader dl;
  PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
  vk::DispatchLoaderDynamic dld(instance_, vkGetInstanceProcAddr);
  instance_.destroyDebugUtilsMessengerEXT(messenger_, nullptr, dld);

  instance_.destroy();
}

void Context::CreateDevice()
{
  // Choose the first GPU
  physical_device_ = instance_.enumeratePhysicalDevices()[0];

  // Find queues
  const auto queue_family_properties = physical_device_.getQueueFamilyProperties();
  for (int i = 0; i < queue_family_properties.size(); i++)
  {
    if (!graphics_queue_index_ && queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
      graphics_queue_index_ = i;

    // TODO: select unique queue family indices
    if ((!present_queue_index_ || graphics_queue_index_ == present_queue_index_) && physical_device_.getSurfaceSupportKHR(i, surface_))
      present_queue_index_ = i;
  }

  float queue_priority = 1.f;
  vk::DeviceQueueCreateInfo graphics_queue_create_info;
  graphics_queue_create_info
    .setQueueFamilyIndex(graphics_queue_index_.value())
    .setQueueCount(1)
    .setQueuePriorities(queue_priority);

  vk::DeviceQueueCreateInfo present_queue_create_info;
  present_queue_create_info
    .setQueueFamilyIndex(present_queue_index_.value())
    .setQueueCount(1)
    .setQueuePriorities(queue_priority);

  std::vector<vk::DeviceQueueCreateInfo> queue_create_infos = {
    graphics_queue_create_info,
    present_queue_create_info,
  };

  // Device extensions
  std::vector<const char*> extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };

  // Device features
  auto features = physical_device_.getFeatures();

  // Create device
  vk::DeviceCreateInfo device_create_info;
  device_create_info
    .setPEnabledExtensionNames(extensions)
    .setQueueCreateInfos(queue_create_infos)
    .setPEnabledFeatures(&features);

  device_ = physical_device_.createDevice(device_create_info);

  graphics_queue_ = device_.getQueue(graphics_queue_index_.value(), 0);
  present_queue_ = device_.getQueue(graphics_queue_index_.value(), 0);
}

void Context::DestroyDevice()
{
  device_.destroy();
}

std::vector<uint32_t> Context::QueueFamilyIndices() const
{
  return std::vector<uint32_t>{
    graphics_queue_index_.value(),
    present_queue_index_.value(),
  };
}
}
}
