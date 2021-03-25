#ifndef TWOPI_VK_VK_INSTANCE_H_
#define TWOPI_VK_VK_INSTANCE_H_

#include <memory>
#include <vector>

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class PhysicalDevice;

class Instance
{
public:
  static std::vector<vk::ExtensionProperties> Extensions();
  static std::vector<vk::LayerProperties> Layers();

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
    bool IsValidationLayerSupported();

    vk::ApplicationInfo app_info_{};
    vk::InstanceCreateInfo create_info_{};

    bool enable_validation_layer_ = false;
    std::vector<std::string> extensions_;
    std::vector<std::string> layers_;
  };

public:
  Instance();
  Instance(vk::Instance instance);

  ~Instance();

  void Destroy();

  operator vk::Instance() const;

  std::vector<PhysicalDevice> PhysicalDevices();

private:
  vk::Instance instance_;
  vk::DebugUtilsMessengerEXT messenger_;
};
}
}

#endif // TWOPI_VK_VK_INSTANCE_H_
