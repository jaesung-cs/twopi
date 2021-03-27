#ifndef TWOPI_VK_VK_SEMAPHORE_H_
#define TWOPI_VK_VK_SEMAPHORE_H_

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Device;

class Semaphore
{
public:
  class Creator
  {
  public:
    Creator() = delete;
    explicit Creator(Device device);
    ~Creator();

    Semaphore Create();

  private:
    const vk::Device device_;

    vk::SemaphoreCreateInfo create_info_{};
  };

public:
  Semaphore();
  Semaphore(vk::Device device, vk::Semaphore semaphore);

  ~Semaphore();

  void Destroy();

  operator vk::Semaphore() const;

private:
  vk::Device device_;
  vk::Semaphore semaphore_;
};
}
}

#endif // TWOPI_VK_VK_SEMAPHORE_H_
