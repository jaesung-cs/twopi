#include <twopi/vk/vk_debug_utils_messenger.h>

#include <iostream>

#include <twopi/vk/vk_instance.h>

namespace twopi
{
namespace vkw
{
//
// Creator
//
DebugUtilsMessenger::Creator::Creator(vk::Instance instance)
{
  create_info_
    .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose)
    .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
    .setPUserData(nullptr);
}

DebugUtilsMessenger::Creator::~Creator() = default;

DebugUtilsMessenger::Creator& DebugUtilsMessenger::Creator::SetCallback(PFN_vkDebugUtilsMessengerCallbackEXT callback)
{
  create_info_.setPfnUserCallback(callback);
  return *this;
}

DebugUtilsMessenger DebugUtilsMessenger::Creator::Create()
{
  DebugUtilsMessenger messenger{ instance_, handle };
  return messenger;
}

//
// DebugUtilsMessenger
//
DebugUtilsMessenger::DebugUtilsMessenger()
{
}

DebugUtilsMessenger::DebugUtilsMessenger(vk::Instance instance, vk::DebugUtilsMessengerEXT messenger)
  : instance_(instance), messenger_(messenger)
{
}

DebugUtilsMessenger::~DebugUtilsMessenger() = default;

void DebugUtilsMessenger::Destroy()
{
}

DebugUtilsMessenger::operator vk::DebugUtilsMessengerEXT() const
{
  return messenger_;
}
}
}
