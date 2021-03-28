#ifndef TWOPI_VK_VK_COMMAND_POOL_H_
#define TWOPI_VK_VK_COMMAND_POOL_H_

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Device;
class Queue;

class CommandPool
{
public:
  class Creator
  {
  public:
    Creator() = delete;
    explicit Creator(Device device);
    ~Creator();

    Creator& SetTransient();
    Creator& SetQueue(Queue queue);

    CommandPool Create();

  private:
    const vk::Device device_;

    vk::CommandPoolCreateInfo create_info_{};
  };

public:
  CommandPool();
  CommandPool(vk::Device device, vk::CommandPool command_pool);

  ~CommandPool();

  void Destroy();

  operator vk::CommandPool() const;

private:
  vk::Device device_;
  vk::CommandPool command_pool_;
};
}
}

#endif // TWOPI_VK_VK_COMMAND_POOL_H_
