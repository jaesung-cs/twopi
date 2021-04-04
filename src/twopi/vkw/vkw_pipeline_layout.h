#ifndef TWOPI_VKW_VKW_PIPELINE_LAYOUT_H_
#define TWOPI_VKW_VKW_PIPELINE_LAYOUT_H_

#include <vector>

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Device;
class DescriptorSetLayout;

class PipelineLayout
{
public:
  class Creator
  {
  public:
    Creator() = delete;
    Creator(Device device);
    ~Creator();

    Creator& SetLayouts(std::vector<DescriptorSetLayout> layouts);

    PipelineLayout Create();

  private:
    const vk::Device device_;

    vk::PipelineLayoutCreateInfo create_info_{};
    std::vector<vk::DescriptorSetLayout> layouts_;
  };

public:
  PipelineLayout();
  PipelineLayout(vk::Device device, vk::PipelineLayout pipeline_layout);

  ~PipelineLayout();
  
  void Destroy();

  operator vk::PipelineLayout() const;

private:
  vk::Device device_;
  vk::PipelineLayout pipeline_layout_;
};
}
}

#endif // TWOPI_VKW_VKW_PIPELINE_LAYOUT_H_
