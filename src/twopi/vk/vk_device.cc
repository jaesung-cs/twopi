#include <twopi/vk/vk_device.h>

#include <twopi/core/error.h>
#include <twopi/vk/internal/vk_device_handle.h>
#include <twopi/vk/vk_physical_device.h>

namespace twopi
{
namespace vk
{
//
// Creator
//
class Device::Creator::Impl
{
public:
  Impl() = delete;

  explicit Impl(PhysicalDevice physical_device)
    : physical_device_(physical_device)
  {
    queue_create_info_.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info_.queueFamilyIndex = 0; // TODO
    queue_create_info_.queueCount = 1;
    queue_create_info_.pQueuePriorities = &queue_priority_;

    create_info_.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info_.pQueueCreateInfos = &queue_create_info_;
    create_info_.queueCreateInfoCount = 1;

    create_info_.pEnabledFeatures = &physical_device_.Features();

    create_info_.enabledExtensionCount = 0;
    create_info_.enabledLayerCount = 0;
  }

  ~Impl()
  {
  }

  Device Create()
  {
    VkDevice handle;
    if (vkCreateDevice(physical_device_, &create_info_, nullptr, &handle) != VK_SUCCESS)
      throw core::Error("Failed to create device");

    return Device{ physical_device_, handle };
  }

private:
  const PhysicalDevice physical_device_;

  VkDeviceCreateInfo create_info_{};
  VkDeviceQueueCreateInfo queue_create_info_{};
  float queue_priority_ = 1.f;
};

Device::Creator::Creator(PhysicalDevice physical_device)
  : impl_(std::make_unique<Impl>(physical_device))
{
}

Device::Creator::~Creator() = default;

Device Device::Creator::Create()
{
  return impl_->Create();
}

//
// Device
//
class Device::Impl
{
public:
  Impl()
  {
  }

  Impl(VkPhysicalDevice physical_device, VkDevice device)
    : device_(std::make_shared<internal::DeviceHandle>(physical_device, device))
  {
  }

  ~Impl()
  {
  }

  operator VkDevice() const { return *device_; }

private:
  std::shared_ptr<internal::DeviceHandle> device_;
};

Device::Device()
  : impl_(std::make_unique<Impl>())
{
}

Device::Device(VkPhysicalDevice physical_device, VkDevice device)
  : impl_(std::make_unique<Impl>(physical_device, device))
{
}

Device::Device(const Device& rhs)
  : impl_(std::make_unique<Impl>(*rhs.impl_))
{
}

Device& Device::operator = (const Device& rhs)
{
  impl_ = std::make_unique<Impl>(*rhs.impl_);
  return *this;
}

Device::Device(Device&& rhs) noexcept = default;

Device& Device::operator = (Device&& rhs) noexcept = default;

Device::~Device() = default;

Device::operator VkDevice() const
{
  return *impl_;
}
}
}
