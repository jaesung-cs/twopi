#ifndef TWOPI_VK_VK_DEBUG_UTILS_MESSENGER_H_
#define TWOPI_VK_VK_DEBUG_UTILS_MESSENGER_H_

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

namespace twopi
{
namespace vk
{
class Instance;

class DebugUtilsMessenger
{
public:
  class Creator
  {
  public:
    Creator() = delete;
    explicit Creator(Instance instance);
    ~Creator();

    DebugUtilsMessenger Create();

  private:
    class Impl;
    std::unique_ptr<Impl> impl_;
  };

public:
  DebugUtilsMessenger();
  DebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger);

  DebugUtilsMessenger(const DebugUtilsMessenger& rhs);
  DebugUtilsMessenger& operator = (const DebugUtilsMessenger& rhs);

  DebugUtilsMessenger(DebugUtilsMessenger&& rhs) noexcept;
  DebugUtilsMessenger& operator = (DebugUtilsMessenger&& rhs) noexcept;

  ~DebugUtilsMessenger();

  operator VkDebugUtilsMessengerEXT() const;

private:
  void SetDependency(Instance instance);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_VK_VK_DEBUG_UTILS_MESSENGER_H_
