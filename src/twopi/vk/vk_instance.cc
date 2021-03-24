#include <twopi/vk/vk_instance.h>

#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <twopi/core/error.h>
#include <twopi/vk/internal/vk_instance_handle.h>
#include <twopi/vk/vk_debug_utils_messenger.h>

namespace twopi
{
namespace vk
{
std::vector<VkExtensionProperties> Instance::Extensions()
{
  uint32_t num_extensions = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &num_extensions, nullptr);
  std::vector<VkExtensionProperties> extensions(num_extensions);
  vkEnumerateInstanceExtensionProperties(nullptr, &num_extensions, extensions.data());
  return extensions;
}

std::vector<VkLayerProperties> Instance::Layers()
{
  uint32_t num_layers;
  vkEnumerateInstanceLayerProperties(&num_layers, nullptr);
  std::vector<VkLayerProperties> layers(num_layers);
  vkEnumerateInstanceLayerProperties(&num_layers, layers.data());
  return layers;
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
    std::cerr << "validation layer: " << callback_data->pMessage << std::endl;

  return VK_FALSE;
}
}

//
// Creator
//
class Instance::Creator::Impl
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

    for (uint32_t i = 0; i < num_glfw_extensions; i++)
      extensions_.push_back(glfw_extensions[i]);
  }

  void EnableValidationLayer()
  {
    if (IsValidationLayerSupported())
    {
      enable_validation_layer_ = true;
      layers_.push_back(validation_layer_name);
      extensions_.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
  }

  Instance Create()
  {
    std::vector<const char*> layer_names;
    for (const auto& layer : layers_)
      layer_names.push_back(layer.c_str());
    create_info_.enabledLayerCount = layer_names.size();
    create_info_.ppEnabledLayerNames = layer_names.data();

    std::vector<const char*> extension_names;
    for (const auto& extension : extensions_)
      extension_names.push_back(extension.c_str());
    create_info_.enabledExtensionCount = extension_names.size();
    create_info_.ppEnabledExtensionNames = extension_names.data();

    VkDebugUtilsMessengerCreateInfoEXT messenger_create_info{};
    if (enable_validation_layer_)
    {
      messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      messenger_create_info.pfnUserCallback = debug_callback;
      messenger_create_info.pUserData = nullptr;

      create_info_.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&messenger_create_info;
    }

    VkInstance handle;
    if (vkCreateInstance(&create_info_, nullptr, &handle)  != VK_SUCCESS)
      throw core::Error("Failed to create vk instance");

    auto instance = Instance{ handle };

    if (enable_validation_layer_)
    {
      auto messenger = DebugUtilsMessenger::Creator{ instance }.Create();
      instance.SetDebugUtilsMessenger(messenger);
    }

    return instance;
  }

private:
  bool IsValidationLayerSupported()
  {
    const auto layers = Instance::Layers();

    const auto it = std::find_if(layers.begin(), layers.end(), [&layers](const VkLayerProperties& layer) {
      return layer.layerName == validation_layer_name;
      });

    return it != layers.cend();
  }

  VkApplicationInfo app_info_{};
  VkInstanceCreateInfo create_info_{};

  bool enable_validation_layer_ = false;
  std::vector<std::string> extensions_;
  std::vector<std::string> layers_;
};

Instance::Creator::Creator()
{
  impl_ = std::make_unique<Impl>();
}

Instance::Creator::~Creator() = default;

Instance Instance::Creator::Create()
{
  return impl_->Create();
}

Instance::Creator& Instance::Creator::AddGlfwRequiredExtensions()
{
  impl_->AddGlfwRequiredExtensions();
  return *this;
}

Instance::Creator& Instance::Creator::EnableValidationLayer()
{
  impl_->EnableValidationLayer();
  return *this;
}

//
// Instance
//
class Instance::Impl
{
public:
  Impl()
  {
  }

  Impl(VkInstance instance)
    : instance_(std::make_shared<internal::InstanceHandle>(instance))
  {
  }

  ~Impl()
  {
  }

  operator VkInstance() const { return *instance_; }

  void SetDebugUtilsMessenger(DebugUtilsMessenger messenger)
  {
    messenger_ = messenger;
  }

private:
  std::shared_ptr<internal::InstanceHandle> instance_;

  // Internally created messenger, lose reference count of actual messenger handle for it to be removed before destroying instance
  DebugUtilsMessenger messenger_;
};

Instance::Instance()
  : impl_(std::make_unique<Impl>())
{
}

Instance::Instance(VkInstance instance)
  : impl_(std::make_unique<Impl>(instance))
{
}

Instance::Instance(const Instance& rhs)
  : impl_(std::make_unique<Impl>(*rhs.impl_))
{
}

Instance& Instance::operator = (const Instance& rhs)
{
  impl_ = std::make_unique<Impl>(*rhs.impl_);
  return *this;
}

Instance::Instance(Instance&& rhs) noexcept = default;

Instance& Instance::operator = (Instance&& rhs) noexcept = default;

Instance::~Instance() = default;

Instance::operator VkInstance() const
{
  return *impl_;
}

void Instance::SetDebugUtilsMessenger(DebugUtilsMessenger messenger)
{
  impl_->SetDebugUtilsMessenger(messenger);
}
}
}
