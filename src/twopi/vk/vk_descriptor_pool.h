#ifndef TWOPI_VK_VK_DESCRIPTOR_POOL_H_
#define TWOPI_VK_VK_DESCRIPTOR_POOL_H_

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Device;

class DescriptorPool
{
public:
  class Creator
  {
  public:
    Creator() = delete;
    explicit Creator(Device device);
    ~Creator();

    Creator& SetSize(uint32_t size);

    DescriptorPool Create();

  private:
    const vk::Device device_;

    vk::DescriptorPoolCreateInfo create_info_{};
    vk::DescriptorPoolSize pool_size_{};
  };

public:
  DescriptorPool();
  DescriptorPool(vk::Device device, vk::DescriptorPool descriptor_pool);

  ~DescriptorPool();

  void Destroy();

  operator vk::DescriptorPool() const;

private:
  vk::Device device_;
  vk::DescriptorPool descriptor_pool_;
};
}
}

#endif // TWOPI_VK_VK_DESCRIPTOR_POOL_H_
