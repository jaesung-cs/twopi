#include <twopi/vke/vke_context.h>

#include <iostream>

#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <twopi/vke/vke_memory_manager.h>

namespace twopi
{
namespace vke
{
namespace
{
const std::string validation_layer_name = "VK_LAYER_KHRONOS_validation";

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

Context::Context(GLFWwindow* window)
{
  const auto available_extensions = vk::enumerateInstanceExtensionProperties();
  std::cout << "Available instance extensions:" << std::endl;
  for (const auto& extension : available_extensions)
    std::cout << "  " << extension.extensionName << std::endl;
  std::cout << std::endl;

  const auto available_layers = vk::enumerateInstanceLayerProperties();
  std::cout << "Available instance layers:" << std::endl;
  for (const auto& layer : available_layers)
    std::cout << "  " << layer.layerName << ": " << layer.description << std::endl;
  std::cout << std::endl;

  vk::ApplicationInfo app_info;
  app_info
    .setPApplicationName("Twopi")
    .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
    .setPEngineName("Twopi Engine")
    .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
    .setApiVersion(VK_API_VERSION_1_2);

  std::vector<const char*> extensions;
  uint32_t num_glfw_extensions = 0;
  const char** glfw_extensions;
  glfw_extensions = glfwGetRequiredInstanceExtensions(&num_glfw_extensions);
  for (uint32_t i = 0; i < num_glfw_extensions; i++)
    extensions.push_back(glfw_extensions[i]);
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  std::vector<const char*> layers;
  layers.push_back(validation_layer_name.c_str());

  vk::DebugUtilsMessengerCreateInfoEXT messenger_create_info;
  messenger_create_info
    .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose)
    .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
    .setPfnUserCallback(debug_callback)
    .setPUserData(nullptr);

  vk::InstanceCreateInfo create_info;
  create_info
    .setPApplicationInfo(&app_info)
    .setEnabledLayerCount(0)
    .setPEnabledLayerNames(layers)
    .setPEnabledExtensionNames(extensions)
    .setPNext(&messenger_create_info);

  instance_ = vk::createInstance(create_info);

  vk::DynamicLoader dl;
  PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
  vk::DispatchLoaderDynamic dld(instance_, vkGetInstanceProcAddr);
  messenger_ = instance_.createDebugUtilsMessengerEXT(messenger_create_info, nullptr, dld);

  // Pick the first physical device
  physical_device_ = instance_.enumeratePhysicalDevices()[0];

  // Create surface
  VkSurfaceKHR surface_handle;
  glfwCreateWindowSurface(instance_, window, nullptr, &surface_handle);
  surface_ = surface_handle;

  // Create device
  std::vector<const char*> device_extensions;
  device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  const auto queue_family_priorities = physical_device_.getQueueFamilyProperties();
  int graphics_family_index = -1;
  int present_family_index = -1;
  for (int i = 0; i < queue_family_priorities.size(); i++)
  {
    if (graphics_family_index == -1 && (queue_family_priorities[i].queueFlags & vk::QueueFlagBits::eGraphics))
      graphics_family_index = i;
    else if (present_family_index == -1 && physical_device_.getSurfaceSupportKHR(i, surface_))
      present_family_index = i;
  }

  float queue_priority = 1.f;
  std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
  vk::DeviceQueueCreateInfo queue_create_info;
  queue_create_info
    .setQueueCount(1)
    .setQueuePriorities(queue_priority)
    .setQueueFamilyIndex(graphics_family_index);
  queue_create_infos.emplace_back(queue_create_info);

  queue_create_info
    .setQueueFamilyIndex(present_family_index);
  queue_create_infos.emplace_back(queue_create_info);

  auto features = physical_device_.getFeatures();
  // TODO: Enable features

  vk::DeviceCreateInfo device_create_info;
  device_create_info
    .setPEnabledExtensionNames(device_extensions)
    .setQueueCreateInfos(queue_create_infos)
    .setPEnabledFeatures(&features);
  device_ = physical_device_.createDevice(device_create_info);

  graphics_queue_ = device_.getQueue(graphics_family_index, 0);
  present_queue_ = device_.getQueue(present_family_index, 0);

  memory_manager_ = std::make_shared<MemoryManagerType>(*this);
}

Context::~Context()
{
  device_.waitIdle();

  device_.destroy();

  vk::DynamicLoader dl;
  PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
  vk::DispatchLoaderDynamic dld(instance_, vkGetInstanceProcAddr);

  instance_.destroyDebugUtilsMessengerEXT(messenger_, nullptr, dld);

  instance_.destroy();
}

vk::PhysicalDevice Context::PhysicalDevice() const
{
  return physical_device_;
}

vk::Device Context::Device() const
{
  return device_;
}

std::shared_ptr<MemoryManager> Context::MemoryManager() const
{
  return memory_manager_;
}
}
}
