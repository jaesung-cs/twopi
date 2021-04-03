#include <twopi/vk/vk_pipeline_layout.h>

#include <twopi/vk/vk_device.h>
#include <twopi/vk/vk_descriptor_set_layout.h>

namespace twopi
{
namespace vkw
{
//
// Creator
//
PipelineLayout::Creator::Creator(Device device)
  : device_(device)
{
  create_info_
    .setPushConstantRanges({});
}

PipelineLayout::Creator::~Creator() = default;

PipelineLayout::Creator& PipelineLayout::Creator::SetLayouts(std::vector<DescriptorSetLayout> layouts)
{
  layouts_ = decltype(layouts_)(layouts.cbegin(), layouts.cend());
  return *this;
}

PipelineLayout PipelineLayout::Creator::Create()
{
  create_info_
    .setSetLayouts(layouts_);

  auto handle = device_.createPipelineLayout(create_info_);
  return PipelineLayout{ device_, handle };
}

//
// PipelineLayout
//
PipelineLayout::PipelineLayout()
{
}

PipelineLayout::PipelineLayout(vk::Device device, vk::PipelineLayout pipeline_layout)
  : device_(device), pipeline_layout_(pipeline_layout)
{
}

PipelineLayout::~PipelineLayout() = default;

void PipelineLayout::Destroy()
{
  device_.destroyPipelineLayout(pipeline_layout_);
}

PipelineLayout::operator vk::PipelineLayout() const
{
  return pipeline_layout_;
}
}
}
