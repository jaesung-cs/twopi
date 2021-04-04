#ifndef TWOPI_VKW_VKW_DEVICE_H_
#define TWOPI_VKW_VKW_DEVICE_H_

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
class Image;
class Buffer;

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
    Creator& AddPortabilitySubsetExtension();

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
  std::pair<uint32_t, vk::Result> AcquireNextImage(Swapchain swapchain, Semaphore semaphore);

  vk::MemoryRequirements MemoryRequirements(Image image) const;
  vk::MemoryRequirements MemoryRequirements(Buffer buffer) const;

  void WaitIdle();

private:
  vk::Device device_;
  std::vector<QueueIndex> queue_indices_;
};
}
}

#endif // TWOPI_VKW_VKW_DEVICE_H_
