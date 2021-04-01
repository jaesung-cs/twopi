#include <twopi/gpu/gpu_device.h>

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace gpu
{
class Device::Impl
{
public:
  Impl(vk::Instance instance, vk::SurfaceKHR surface)
    : surface_(surface)
  {
    // Create device
    const auto physical_devices = instance.enumeratePhysicalDevices();
    physical_device_ = physical_devices[0];

    const auto queue_family_properties = physical_device_.getQueueFamilyProperties();
    for (int i = 0; i < queue_family_properties.size(); i++)
    {
      if (graphics_queue_family_ == -1 && queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
        graphics_queue_family_ = i;

      // TODO: select unique queue family indices
      if ((present_queue_family_ == -1 || present_queue_family_ == graphics_queue_family_) && physical_device_.getSurfaceSupportKHR(i, surface))
        present_queue_family_ = i;
    }

    std::vector<const char*> device_extensions{
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    uint32_t family_index = 0;
    std::vector<float> priorities{ 1.f };
    // Assume graphics and present indices are different
    std::vector<vk::DeviceQueueCreateInfo> queue_create_info{
      { {}, graphics_queue_family_, priorities },
      { {}, present_queue_family_, priorities },
    };
    device_ = physical_device_.createDevice({
      {}, queue_create_info, {}, device_extensions, {}
      });
    graphics_queue_ = device_.getQueue(graphics_queue_family_, 0);
    present_queue_ = device_.getQueue(present_queue_family_, 0);
  }

  ~Impl()
  {
    device_.destroy();
  }

  vk::PhysicalDevice PhysicalDevice() const
  {
    return physical_device_;
  }

  vk::Device DeviceHandle() const
  {
    return device_;
  }

  vk::SurfaceKHR Surface() const
  {
    return surface_;
  }

  std::vector<uint32_t> QueueFamilies() const
  {
    return { graphics_queue_family_, present_queue_family_ };
  }

private:
  const vk::SurfaceKHR surface_;
  vk::PhysicalDevice physical_device_;

  vk::Device device_;
  vk::Queue graphics_queue_;
  vk::Queue present_queue_;
  uint32_t graphics_queue_family_;
  uint32_t present_queue_family_;
};

Device::Device(vk::Instance instance, vk::SurfaceKHR surface)
  : impl_(std::make_unique<Impl>(instance, surface))
{
}

Device::~Device() = default;

vk::PhysicalDevice Device::PhysicalDevice() const
{
  return impl_->PhysicalDevice();
}

vk::Device Device::DeviceHandle() const
{
  return impl_->DeviceHandle();
}

vk::SurfaceKHR Device::Surface() const
{
  return impl_->Surface();
}

std::vector<uint32_t> Device::QueueFamilies() const
{
  return impl_->QueueFamilies();
}
}
}
