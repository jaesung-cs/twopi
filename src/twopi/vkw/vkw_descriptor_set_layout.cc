#include <twopi/vkw/vkw_descriptor_set_layout.h>

#include <twopi/vkw/vkw_device.h>

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
}

DescriptorSetLayout::Creator::~Creator() = default;

DescriptorSetLayout::Creator& DescriptorSetLayout::Creator::AddUniformBuffer()
{
  vk::DescriptorSetLayoutBinding binding;
  binding
    .setBinding(bindings_.size())
    .setDescriptorType(vk::DescriptorType::eUniformBuffer)
    .setStageFlags(vk::ShaderStageFlagBits::eVertex)
    .setDescriptorCount(1);

  bindings_.emplace_back(std::move(binding));

  return *this;
}

DescriptorSetLayout::Creator& DescriptorSetLayout::Creator::AddSampler()
{
  vk::DescriptorSetLayoutBinding binding;
  binding
    .setBinding(bindings_.size())
    .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
    .setStageFlags(vk::ShaderStageFlagBits::eFragment)
    .setImmutableSamplers({})
    .setDescriptorCount(1);

  bindings_.emplace_back(std::move(binding));

  return *this;
}

DescriptorSetLayout DescriptorSetLayout::Creator::Create()
{
  create_info_
    .setBindings(bindings_);

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
