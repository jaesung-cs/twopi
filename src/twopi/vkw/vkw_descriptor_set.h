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
  struct Uniform
  {
    enum class Type
    {
      BUFFER,
      SAMPLER,
    };

    Uniform() = delete;
    Uniform(vk::Buffer buffer, uint64_t offset = 0, uint64_t size = VK_WHOLE_SIZE)
      : type(Type::BUFFER), buffer(buffer), offset(offset), size(size) { }

    Uniform(vk::ImageView image_view, vk::Sampler sampler)
      : type(Type::SAMPLER), image_view(image_view), sampler(sampler) { }

    Type type;

    vk::Buffer buffer;
    uint64_t offset;
    uint64_t size;

    vk::ImageView image_view;
    vk::Sampler sampler;
  };

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

  void Update(std::initializer_list<Uniform> uniforms);

private:
  vk::Device device_;
  vk::DescriptorSet descriptor_set_;
};
}
}

#endif // TWOPI_VKW_VKW_DESCRIPTOR_SET_H_
