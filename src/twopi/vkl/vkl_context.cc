#include <twopi/vkl/vkl_context.h>

#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <twopi/vkl/vkl_memory.h>
#include <twopi/vkl/vkl_memory_manager.h>

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
  CreateCommandPools();

  memory_manager_ = std::make_unique<MemoryManager>(this);
}

Context::~Context()
{
  memory_manager_.reset();

  DestroyCommandPools();
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
  constexpr auto queue_flag = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute;
  const auto queue_family_properties = physical_device_.getQueueFamilyProperties();
  for (int i = 0; i < queue_family_properties.size(); i++)
  {
    if (!queue_index_ &&
      (queue_family_properties[i].queueFlags & queue_flag) == queue_flag &&
      physical_device_.getSurfaceSupportKHR(i, surface_) &&
      queue_family_properties[i].queueCount >= 2)
    {
      queue_index_ = i;
      break;
    }
  }

  std::vector<float> queue_priorities = {
    1.f, 1.f
  };
  vk::DeviceQueueCreateInfo queue_create_info;
  queue_create_info
    .setQueueFamilyIndex(queue_index_.value())
    .setQueueCount(2)
    .setQueuePriorities(queue_priorities);

  // Device extensions
  std::vector<const char*> extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };

  // Device features
  auto features = physical_device_.getFeatures();
  features
    .setTessellationShader(true)
    .setGeometryShader(true);

  // Create device
  vk::DeviceCreateInfo device_create_info;
  device_create_info
    .setPEnabledExtensionNames(extensions)
    .setQueueCreateInfos(queue_create_info)
    .setPEnabledFeatures(&features);

  device_ = physical_device_.createDevice(device_create_info);

  queue_ = device_.getQueue(queue_index_.value(), 0);
  present_queue_ = device_.getQueue(queue_index_.value(), 1);
}

void Context::DestroyDevice()
{
  device_.destroy();
}

void Context::CreateCommandPools()
{
  // Create command pools
  vk::CommandPoolCreateInfo command_pool_create_info;
  command_pool_create_info
    .setQueueFamilyIndex(queue_index_.value())
    .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

  command_pool_ = device_.createCommandPool(command_pool_create_info);

  command_pool_create_info
    .setFlags(vk::CommandPoolCreateFlagBits::eTransient);

  transient_command_pool_ = device_.createCommandPool(command_pool_create_info);
}

void Context::DestroyCommandPools()
{
  device_.destroyCommandPool(command_pool_);
  device_.destroyCommandPool(transient_command_pool_);
}

std::vector<uint32_t> Context::QueueFamilyIndices() const
{
  return std::vector<uint32_t>{
    queue_index_.value(),
  };
}

Memory Context::AllocateDeviceMemory(vk::Buffer buffer)
{
  return memory_manager_->AllocateDeviceMemory(buffer);
}

Memory Context::AllocateDeviceMemory(vk::Image image)
{
  return memory_manager_->AllocateDeviceMemory(image);
}

Memory Context::AllocateHostMemory(vk::Buffer buffer)
{
  return memory_manager_->AllocateHostMemory(buffer);
}

Memory Context::AllocatePersistentlyMappedMemory(vk::Buffer buffer)
{
  return memory_manager_->AllocatePersistentlyMappedMemory(buffer);
}

std::vector<vk::CommandBuffer> Context::AllocateCommandBuffers(int count)
{
  vk::CommandBufferAllocateInfo allocate_info;
  allocate_info
    .setLevel(vk::CommandBufferLevel::ePrimary)
    .setCommandPool(command_pool_)
    .setCommandBufferCount(count);
  return device_.allocateCommandBuffers(allocate_info);
}

std::vector<vk::CommandBuffer> Context::AllocateTransientCommandBuffers(int count)
{
  vk::CommandBufferAllocateInfo allocate_info;
  allocate_info
    .setLevel(vk::CommandBufferLevel::ePrimary)
    .setCommandPool(transient_command_pool_)
    .setCommandBufferCount(count);
  return device_.allocateCommandBuffers(allocate_info);
}

void Context::FreeCommandBuffers(std::vector<vk::CommandBuffer>&& command_buffers)
{
  for (auto& command_buffer : command_buffers)
    device_.freeCommandBuffers(command_pool_, command_buffer);
  command_buffers.clear();
}

void Context::FreeTransientCommandBuffers(std::vector<vk::CommandBuffer>&& command_buffers)
{
  for (auto& command_buffer : command_buffers)
    device_.freeCommandBuffers(transient_command_pool_, command_buffer);
  command_buffers.clear();
}
}
}
