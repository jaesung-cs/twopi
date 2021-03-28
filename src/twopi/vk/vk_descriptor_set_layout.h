#ifndef TWOPI_VK_VK_DESCRIPTOR_SET_LAYOUT_H_
#define TWOPI_VK_VK_DESCRIPTOR_SET_LAYOUT_H_

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Device;

class DescriptorSetLayout
{
public:
  class Creator
  {
  public:
    Creator() = delete;
    explicit Creator(Device device);
    ~Creator();

    DescriptorSetLayout Create();

  private:
    const vk::Device device_;

    vk::DescriptorSetLayoutCreateInfo create_info_{};
    vk::DescriptorSetLayoutBinding binding_{};
  };

public:
  DescriptorSetLayout();
  DescriptorSetLayout(vk::Device device, vk::DescriptorSetLayout descriptor_set_layout);

  ~DescriptorSetLayout();

  void Destroy();

  operator vk::DescriptorSetLayout() const;

private:
  vk::Device device_;
  vk::DescriptorSetLayout descriptor_set_layout_;
};
}
}

#endif // TWOPI_VK_VK_DESCRIPTOR_SET_LAYOUT_H_
