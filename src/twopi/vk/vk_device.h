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
class Surface;
class Queue;
class Swapchain;
class Semaphore;

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
      PRESENT,
    };

  public:
    Creator() = delete;
    explicit Creator(PhysicalDevice physical_device);
    ~Creator();

    Creator& AddGraphicsQueue();
    Creator& AddPresentQueue(Surface surface);
    Creator& AddSwapchainExtension();

    Device Create();

  private:
    vk::PhysicalDevice physical_device_;

    vk::SurfaceKHR surface_;
    std::vector<QueueType> queue_types_;

    vk::DeviceCreateInfo create_info_{};

    std::vector<std::string> extensions_;
  };

public:
  Device();
  Device(vk::Device device);

  ~Device();

  void Destroy();

  operator vk::Device() const;

  vkw::Queue Queue(int index) const;
  uint32_t AcquireNextImage(Swapchain swapchain, Semaphore semaphore);

  void WaitIdle();

private:
  vk::Device device_;
  std::vector<QueueIndex> queue_indices_;
};
}
}

#endif // TWOPI_VK_VK_DEVICE_H_
