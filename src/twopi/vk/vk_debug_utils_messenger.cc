#include <twopi/vk/vk_debug_utils_messenger.h>

#include <iostream>

#include <twopi/core/error.h>
#include <twopi/vk/internal/vk_debug_utils_messenger_handle.h>
#include <twopi/vk/vk_instance.h>

namespace twopi
{
namespace vk
{
namespace
{
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr)
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  else
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

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
class DebugUtilsMessenger::Creator::Impl
{
public:
  Impl(Instance instance)
    : instance_(instance)
  {
    create_info_.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info_.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info_.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info_.pfnUserCallback = debug_callback;
    create_info_.pUserData = nullptr;
  }

  ~Impl()
  {
  }

  DebugUtilsMessenger Create()
  {
    VkDebugUtilsMessengerEXT debug_messenger;
    if (CreateDebugUtilsMessengerEXT(instance_, &create_info_, nullptr, &debug_messenger) != VK_SUCCESS)
      throw std::runtime_error("Failed to set up debug messenger");

    auto messenger = DebugUtilsMessenger{ instance_, debug_messenger };
    messenger.SetDependency();
    return messenger;
  }

private:
  VkDebugUtilsMessengerCreateInfoEXT create_info_{};

  const Instance instance_;
};

DebugUtilsMessenger::Creator::Creator(Instance instance)
{
  impl_ = std::make_unique<Impl>(instance);
}

DebugUtilsMessenger::Creator::~Creator() = default;

DebugUtilsMessenger DebugUtilsMessenger::Creator::Create()
{
  return impl_->Create();
}

//
// DebugUtilsMessenger
//
class DebugUtilsMessenger::Impl
{
public:
  Impl()
  {
  }

  Impl(VkInstance instance, VkDebugUtilsMessengerEXT messenger)
    : messenger_(std::make_shared<internal::DebugUtilsMessengerHandle>(instance, messenger))
  {
  }

  ~Impl()
  {
  }

  operator VkDebugUtilsMessengerEXT() const { return *messenger_; }

  void SetDependency(Instance instance)
  {
    messenger_->SetDependency(instance);
  }

private:
  std::shared_ptr<internal::DebugUtilsMessengerHandle> messenger_;
};

DebugUtilsMessenger::DebugUtilsMessenger()
  : impl_(std::make_unique<Impl>())
{
}

DebugUtilsMessenger::DebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger)
  : impl_(std::make_unique<Impl>(instance, messenger))
{
}

DebugUtilsMessenger::DebugUtilsMessenger(const DebugUtilsMessenger& rhs)
  : impl_(std::make_unique<Impl>(*rhs.impl_))
{
}

DebugUtilsMessenger& DebugUtilsMessenger::operator = (const DebugUtilsMessenger& rhs)
{
  impl_ = std::make_unique<Impl>(*rhs.impl_);
  return *this;
}

DebugUtilsMessenger::DebugUtilsMessenger(DebugUtilsMessenger&& rhs) noexcept = default;

DebugUtilsMessenger& DebugUtilsMessenger::operator = (DebugUtilsMessenger&& rhs) noexcept = default;

DebugUtilsMessenger::~DebugUtilsMessenger() = default;

DebugUtilsMessenger::operator VkDebugUtilsMessengerEXT() const
{
  return *impl_;
}

void DebugUtilsMessenger::SetDependency(Instance instance)
{
  impl_->SetDependency(instance);
}
}
}
