#include <twopi/vk/vk_instance.h>

#include <iostream>
#include <algorithm>

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <twopi/core/error.h>
#include <twopi/vk/vk_physical_device.h>

namespace twopi
{
namespace vkw
{
std::vector<vk::ExtensionProperties> Instance::Extensions()
{
  return vk::enumerateInstanceExtensionProperties();
}

std::vector<vk::LayerProperties> Instance::Layers()
{
  return vk::enumerateInstanceLayerProperties();
}

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
    std::cerr << callback_data->pMessage << std::endl;

  return VK_FALSE;
}
}

//
// Creator
//
Instance::Creator::Creator()
{
  app_info_
    .setPApplicationName("Twopi")
    .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
    .setPEngineName("Twopi Engine")
    .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
    .setApiVersion(VK_API_VERSION_1_2);

  create_info_
    .setPApplicationInfo(&app_info_)
    .setEnabledLayerCount(0);
}

Instance::Creator::~Creator() = default;

Instance Instance::Creator::Create()
{
  std::vector<const char*> layer_names;
  for (const auto& layer : layers_)
    layer_names.push_back(layer.c_str());
  create_info_.setPEnabledLayerNames(layer_names);

  std::vector<const char*> extension_names;
  for (const auto& extension : extensions_)
    extension_names.push_back(extension.c_str());
  create_info_.setPEnabledExtensionNames(extension_names);

  vk::DebugUtilsMessengerCreateInfoEXT messenger_create_info;
  if (enable_validation_layer_)
  {
    messenger_create_info
      .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose)
      .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
      .setPfnUserCallback(debug_callback)
      .setPUserData(nullptr);

    create_info_.setPNext(&messenger_create_info);
  }

  const auto handle = vk::createInstance(create_info_);
  auto instance = Instance{ handle };

  if (enable_validation_layer_)
  {
    vk::DynamicLoader dl;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    vk::DispatchLoaderDynamic dld(static_cast<vk::Instance>(instance), vkGetInstanceProcAddr);

    instance.messenger_ = handle.createDebugUtilsMessengerEXT(messenger_create_info, nullptr, dld);
  }

  return instance;
}

Instance::Creator& Instance::Creator::AddGlfwRequiredExtensions()
{
  uint32_t num_glfw_extensions = 0;
  const char** glfw_extensions;

  glfw_extensions = glfwGetRequiredInstanceExtensions(&num_glfw_extensions);

  for (uint32_t i = 0; i < num_glfw_extensions; i++)
    extensions_.push_back(glfw_extensions[i]);

  return *this;
}

Instance::Creator& Instance::Creator::EnableValidationLayer()
{
  if (IsValidationLayerSupported())
  {
    enable_validation_layer_ = true;
    layers_.push_back(validation_layer_name);
    extensions_.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return *this;
}

bool Instance::Creator::IsValidationLayerSupported()
{
  const auto layers = Instance::Layers();

  const auto it = std::find_if(layers.begin(), layers.end(), [&layers](const VkLayerProperties& layer) {
    return layer.layerName == validation_layer_name;
    });

  return it != layers.cend();
}

//
// Instance
//
Instance::Instance()
{
}

Instance::Instance(vk::Instance instance)
  : instance_(instance)
{
}

Instance::~Instance() = default;

void Instance::Destroy()
{
  if (messenger_)
  {
    vk::DynamicLoader dl;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    vk::DispatchLoaderDynamic dld(instance_, vkGetInstanceProcAddr);

    instance_.destroyDebugUtilsMessengerEXT(messenger_, nullptr, dld);
  }

  instance_.destroy();
}

Instance::operator vk::Instance() const
{
  return instance_;
}

std::vector<PhysicalDevice> Instance::PhysicalDevices()
{
  const auto physical_device_handles = instance_.enumeratePhysicalDevices();

  std::vector<PhysicalDevice> physical_devices;
  std::transform(physical_device_handles.begin(), physical_device_handles.end(), std::back_inserter(physical_devices),
    [&](vk::PhysicalDevice physical_device_handle) {
      return PhysicalDevice(physical_device_handle);
    });

  return physical_devices;
}
}
}
