#include <twopi/vk/vk_physical_device.h>

#include <twopi/vk/internal/vk_physical_device_handle.h>

namespace twopi
{
namespace vk
{
class PhysicalDevice::Impl
{
public:
  Impl(VkInstance instance, VkPhysicalDevice physical_device)
    : physical_device_(std::make_shared<internal::PhysicalDeviceHandle>(instance, physical_device))
  {
    vkGetPhysicalDeviceProperties(physical_device, &properties_);
    vkGetPhysicalDeviceFeatures(physical_device, &features_);
  }

  ~Impl()
  {
  }

  operator VkPhysicalDevice() const { return *physical_device_; }

  const VkPhysicalDeviceProperties& Properties() const
  {
    return properties_;
  }

  const VkPhysicalDeviceFeatures& Features() const
  {
    return features_;
  }

private:
  std::shared_ptr<internal::PhysicalDeviceHandle> physical_device_;

  VkPhysicalDeviceProperties properties_{};
  VkPhysicalDeviceFeatures features_{};
};

PhysicalDevice::PhysicalDevice(VkInstance instance, VkPhysicalDevice physical_device)
  : impl_(std::make_unique<Impl>(instance, physical_device))
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
