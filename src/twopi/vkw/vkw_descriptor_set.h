#ifndef TWOPI_VKW_VKW_DESCRIPTOR_SET_H_
#define TWOPI_VKW_VKW_DESCRIPTOR_SET_H_

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Device;
class DescriptorPool;
class DescriptorSetLayout;
class Buffer;
class ImageView;
class Sampler;

class DescriptorSet
{
public:
  class Allocator
  {
  public:
    Allocator() = delete;
    Allocator(Device device, DescriptorPool descriptor_pool);
    ~Allocator();

    Allocator& SetLayout(DescriptorSetLayout layout);
    Allocator& SetSize(uint32_t size);

    std::vector<DescriptorSet> Allocate();

  private:
    const vk::Device device_;
    const vk::DescriptorPool descriptor_pool_;

    vk::DescriptorSetAllocateInfo allocate_info_{};
    vk::DescriptorSetLayout layout_handle_{};
    uint32_t size_ = 0;
  };

public:
  DescriptorSet();
  DescriptorSet(vk::Device device, vk::DescriptorSet descriptor_set);

  ~DescriptorSet();

  operator vk::DescriptorSet() const;

  void Update(Buffer buffer, ImageView image_view, Sampler sampler);

private:
  vk::Device device_;
  vk::DescriptorSet descriptor_set_;
};
}
}

#endif // TWOPI_VKW_VKW_DESCRIPTOR_SET_H_
