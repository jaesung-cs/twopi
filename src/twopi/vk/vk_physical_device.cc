#include <twopi/vk/vk_physical_device.h>

namespace twopi
{
namespace vk
{
class PhysicalDevice::Impl
{
public:
  Impl()
  {
  }

  Impl(VkPhysicalDevice physical_device)
    : physical_device_(physical_device)
  {
    vkGetPhysicalDeviceProperties(physical_device, &properties_);
    vkGetPhysicalDeviceFeatures(physical_device, &features_);
  }

  ~Impl()
  {
  }

  operator VkPhysicalDevice() const { return physical_device_; }

  const VkPhysicalDeviceProperties& Properties() const
  {
    return properties_;
  }

  const VkPhysicalDeviceFeatures& Features() const
  {
    return features_;
  }

private:
  VkPhysicalDevice physical_device_ = nullptr;
  VkPhysicalDeviceProperties properties_;
  VkPhysicalDeviceFeatures features_;
};

PhysicalDevice::PhysicalDevice(VkPhysicalDevice physical_device)
  : impl_(std::make_unique<Impl>(physical_device))
{
}

PhysicalDevice::PhysicalDevice(const PhysicalDevice& rhs)
  : impl_(std::make_unique<Impl>(*rhs.impl_))
{
}

PhysicalDevice& PhysicalDevice::operator = (const PhysicalDevice& rhs)
{
  impl_ = std::make_unique<Impl>(*rhs.impl_);
  return *this;
}

PhysicalDevice::PhysicalDevice(PhysicalDevice&& rhs) noexcept = default;

PhysicalDevice& PhysicalDevice::operator = (PhysicalDevice&& rhs) noexcept = default;

PhysicalDevice::~PhysicalDevice() = default;

PhysicalDevice::operator VkPhysicalDevice() const
{
  return *impl_;
}

const VkPhysicalDeviceProperties& PhysicalDevice::Properties() const
{
  return impl_->Properties();
}

const VkPhysicalDeviceFeatures& PhysicalDevice::Features() const
{
  return impl_->Features();
}
}
}
