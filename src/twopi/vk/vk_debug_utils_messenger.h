#ifndef TWOPI_VK_VK_DEBUG_UTILS_MESSENGER_H_
#define TWOPI_VK_VK_DEBUG_UTILS_MESSENGER_H_

#include <memory>
#include <vector>

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class DebugUtilsMessenger
{
public:
  class Creator
  {
  public:
    Creator() = delete;
    explicit Creator(vk::Instance instance);
    ~Creator();

    Creator& SetCallback(PFN_vkDebugUtilsMessengerCallbackEXT callback);

    DebugUtilsMessenger Create();

  private:
    vk::DebugUtilsMessengerCreateInfoEXT create_info_{};

    vk::Instance instance_;
  };

public:
  DebugUtilsMessenger();
  DebugUtilsMessenger(vk::Instance instance, vk::DebugUtilsMessengerEXT messenger);

  ~DebugUtilsMessenger();

  void Destroy();

  operator vk::DebugUtilsMessengerEXT() const;

private:
  vk::DebugUtilsMessengerEXT messenger_;
  vk::Instance instance_;
};
}
}

#endif // TWOPI_VK_VK_DEBUG_UTILS_MESSENGER_H_
