#ifndef TWOPI_VK_VK_INSTANCE_H_
#define TWOPI_VK_VK_INSTANCE_H_

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

namespace twopi
{
namespace vk
{
class DebugUtilsMessenger;
class PhysicalDevice;

class Instance
{
public:
  static std::vector<VkExtensionProperties> Extensions();
  static std::vector<VkLayerProperties> Layers();

public:
  class Creator
  {
  public:
    Creator();
    ~Creator();

    Creator& AddGlfwRequiredExtensions();
    Creator& EnableValidationLayer();

    Instance Create();

  private:
    class Impl;
    std::unique_ptr<Impl> impl_;
  };

public:
  Instance();
  Instance(VkInstance instance);

  Instance(const Instance& rhs);
  Instance& operator = (const Instance& rhs);

  Instance(Instance&& rhs) noexcept;
  Instance& operator = (Instance&& rhs) noexcept;

  ~Instance();

  operator VkInstance() const;

  std::vector<PhysicalDevice> PhysicalDevices();

private:
  void SetDebugUtilsMessenger(DebugUtilsMessenger messenger);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_VK_VK_INSTANCE_H_
