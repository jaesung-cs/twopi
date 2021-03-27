#ifndef TWOPI_VK_VK_SWAPCHAIN_H_
#define TWOPI_VK_VK_SWAPCHAIN_H_

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class PhysicalDevice;
class Device;
class Surface;
class Queue;
class Image;

class Swapchain
{
public:
  class Creator
  {
  public:
    Creator() = delete;
    Creator(PhysicalDevice physical_device, Device device, Surface surface);
    ~Creator();

    Creator& SetTripleBuffering();
    Creator& SetDefaultFormat();
    Creator& SetExtent(int width, int height);
    Creator& SetQueues(std::initializer_list<Queue> queues);

    Swapchain Create();

  private:
    vk::PhysicalDevice physical_device_;
    vk::Device device_;
    vk::SurfaceKHR surface_;

    vk::SurfaceCapabilitiesKHR capabilities_{};

    vk::SwapchainCreateInfoKHR create_info_{};
  };

public:
  Swapchain();
  Swapchain(vk::Device device, vk::SwapchainKHR swapchain);

  ~Swapchain();
  
  void Destroy();

  operator vk::SwapchainKHR() const;

  std::vector<Image> Images() const;

private:
  vk::Device device_;
  vk::SwapchainKHR swapchain_;
};
}
}

#endif // TWOPI_VK_VK_SWAPCHAIN_H_
