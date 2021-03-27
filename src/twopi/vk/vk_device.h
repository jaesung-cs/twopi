#ifndef TWOPI_VK_VK_DEVICE_H_
#define TWOPI_VK_VK_DEVICE_H_

#include <memory>
#include <vector>

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class PhysicalDevice;
class Queue;

class Device
{
private:
  struct QueueIndex
  {
    int family_index;
    int queue_index;
  };

public:
  class Creator
  {
  private:
    enum class QueueType
    {
      GRAPHICS,
    };

  public:
    Creator() = delete;
    explicit Creator(PhysicalDevice physical_device);
    ~Creator();

    Creator& AddGraphicsQueue();

    Device Create();

  private:
    vk::PhysicalDevice physical_device_;

    std::vector<QueueType> queue_types_;

    vk::DeviceCreateInfo create_info_{};
  };

public:
  Device();
  Device(vk::Device device);

  ~Device();

  vkw::Queue Queue(int index);

  void Destroy();

  operator vk::Device() const;

private:
  vk::Device device_;
  std::vector<QueueIndex> queue_indices_;
};
}
}

#endif // TWOPI_VK_VK_DEVICE_H_
