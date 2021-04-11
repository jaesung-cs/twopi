#include <twopi/vkw/vkw_descriptor_set.h>

#include <twopi/vkw/vkw_device.h>
#include <twopi/vkw/vkw_descriptor_pool.h>
#include <twopi/vkw/vkw_descriptor_set_layout.h>
#include <twopi/vkw/vkw_buffer.h>
#include <twopi/vkw/vkw_image_view.h>
#include <twopi/vkw/vkw_sampler.h>

namespace twopi
{
namespace vkw
{
//
// Allocator
//
DescriptorSet::Allocator::Allocator(Device device, DescriptorPool descriptor_pool)
  : device_(device), descriptor_pool_(descriptor_pool)
{
}

DescriptorSet::Allocator::~Allocator() = default;

DescriptorSet::Allocator& DescriptorSet::Allocator::SetLayout(DescriptorSetLayout layout)
{
  layout_handle_ = layout;
  return *this;
}

DescriptorSet::Allocator& DescriptorSet::Allocator::SetSize(uint32_t size)
{
  size_ = size;
  return *this;
}

std::vector<DescriptorSet> DescriptorSet::Allocator::Allocate()
{
  std::vector<vk::DescriptorSetLayout> layout_handles(size_, layout_handle_);
  allocate_info_
    .setDescriptorPool(descriptor_pool_)
    .setSetLayouts(layout_handles);

  const auto handles = device_.allocateDescriptorSets(allocate_info_);
  std::vector<DescriptorSet> descriptor_sets;
  for (auto handle : handles)
    descriptor_sets.emplace_back(device_, handle);
  return descriptor_sets;
}

//
// DescriptorSet
//
DescriptorSet::DescriptorSet()
{
}

DescriptorSet::DescriptorSet(vk::Device device, vk::DescriptorSet descriptor_set)
  : device_(device), descriptor_set_(descriptor_set)
{
}

DescriptorSet::~DescriptorSet() = default;

DescriptorSet::operator vk::DescriptorSet() const
{
  return descriptor_set_;
}

void DescriptorSet::Update(std::initializer_list<Uniform> uniforms)
{
  std::vector<std::pair<int, vk::DescriptorBufferInfo>> buffer_infos;
  std::vector<std::pair<int, vk::DescriptorImageInfo>> image_infos;
  std::vector<vk::WriteDescriptorSet> writes;

  int binding = 0;
  for (const auto& uniform : uniforms)
  {
    switch (uniform.type)
    {
    case Uniform::Type::BUFFER:
    {
      auto buffer = uniform.buffer;
      auto offset = uniform.offset;
      auto size = uniform.size;

      vk::DescriptorBufferInfo buffer_info;
      buffer_info
        .setBuffer(buffer)
        .setOffset(offset)
        .setRange(size);

      buffer_infos.emplace_back(binding, std::move(buffer_info));
    }
      break;
    case Uniform::Type::SAMPLER:
    {
      auto image_view = uniform.image_view;
      auto sampler = uniform.sampler;

      vk::DescriptorImageInfo image_info;
      image_info
        .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
        .setImageView(image_view)
        .setSampler(sampler);

      image_infos.emplace_back(binding, std::move(image_info));
    }
      break;
    }

    binding++;
  }

  for (int i = 0; i < buffer_infos.size(); i++)
  {
    vk::WriteDescriptorSet write;
    write
      .setDescriptorType(vk::DescriptorType::eUniformBuffer)
      .setDescriptorCount(1)
      .setDstSet(descriptor_set_)
      .setDstBinding(buffer_infos[i].first)
      .setDstArrayElement(0)
      .setBufferInfo(buffer_infos[i].second);

    writes.emplace_back(std::move(write));
  }

  for (int i = 0; i < image_infos.size(); i++)
  {
    vk::WriteDescriptorSet write;
    write
      .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
      .setDescriptorCount(1)
      .setDstSet(descriptor_set_)
      .setDstBinding(image_infos[i].first)
      .setDstArrayElement(0)
      .setImageInfo(image_infos[i].second);

    writes.emplace_back(std::move(write));
  }

  device_.updateDescriptorSets(writes, nullptr);
}
}
}
