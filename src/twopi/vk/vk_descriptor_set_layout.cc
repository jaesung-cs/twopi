#include <twopi/vk/vk_descriptor_set_layout.h>

#include <twopi/vk/vk_device.h>

namespace twopi
{
namespace vkw
{
//
// Creator
//
DescriptorSetLayout::Creator::Creator(Device device)
  : device_(device)
{
  binding_
    .setBinding(0)
    .setDescriptorType(vk::DescriptorType::eUniformBuffer)
    .setDescriptorCount(1)
    .setStageFlags(vk::ShaderStageFlagBits::eVertex);
}

DescriptorSetLayout::Creator::~Creator() = default;

DescriptorSetLayout DescriptorSetLayout::Creator::Create()
{
  create_info_.setBindings(binding_);

  const auto handle = device_.createDescriptorSetLayout(create_info_);
  return DescriptorSetLayout{ device_, handle };
}

//
// DescriptorSetLayout
//
DescriptorSetLayout::DescriptorSetLayout()
{
}

DescriptorSetLayout::DescriptorSetLayout(vk::Device device, vk::DescriptorSetLayout descriptor_set_layout)
  : device_(device), descriptor_set_layout_(descriptor_set_layout)
{
}

DescriptorSetLayout::~DescriptorSetLayout() = default;

void DescriptorSetLayout::Destroy()
{
  device_.destroyDescriptorSetLayout(descriptor_set_layout_);
}

DescriptorSetLayout::operator vk::DescriptorSetLayout() const
{
  return descriptor_set_layout_;
}
}
}
