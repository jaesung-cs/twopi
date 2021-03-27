#include <twopi/vk/vk_pipeline_layout.h>

#include <twopi/vk/vk_device.h>

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
    .setSetLayouts({})
    .setPushConstantRanges({});

}

PipelineLayout::Creator::~Creator() = default;

PipelineLayout PipelineLayout::Creator::Create()
{
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
