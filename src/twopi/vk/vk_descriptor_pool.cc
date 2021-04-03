#include <twopi/vk/vk_descriptor_pool.h>

#include <twopi/vk/vk_device.h>

namespace twopi
{
namespace vkw
{
//
// Creator
//
DescriptorPool::Creator::Creator(Device device)
  : device_(device)
{
}

DescriptorPool::Creator::~Creator() = default;

DescriptorPool::Creator& DescriptorPool::Creator::AddUniformBuffer()
{
  vk::DescriptorPoolSize pool_size;
  pool_size.setType(vk::DescriptorType::eUniformBuffer);
  pool_sizes_.emplace_back(std::move(pool_size));

  return *this;
}

DescriptorPool::Creator& DescriptorPool::Creator::AddSampler()
{
  vk::DescriptorPoolSize pool_size;
  pool_size.setType(vk::DescriptorType::eCombinedImageSampler);
  pool_sizes_.emplace_back(std::move(pool_size));

  return *this;
}

DescriptorPool::Creator& DescriptorPool::Creator::SetSize(uint32_t size)
{
  descriptor_count_ = size;
  return *this;
}

DescriptorPool DescriptorPool::Creator::Create()
{
  for (auto& pool_size : pool_sizes_)
    pool_size.setDescriptorCount(descriptor_count_);

  create_info_
    .setPoolSizes(pool_sizes_)
    .setMaxSets(descriptor_count_);

  const auto handle = device_.createDescriptorPool(create_info_);
  return DescriptorPool{ device_, handle };
}

//
// DescriptorPool
//
DescriptorPool::DescriptorPool()
{
}

DescriptorPool::DescriptorPool(vk::Device device, vk::DescriptorPool descriptor_pool)
  : device_(device), descriptor_pool_(descriptor_pool)
{
}

DescriptorPool::~DescriptorPool() = default;

void DescriptorPool::Destroy()
{
  device_.destroyDescriptorPool(descriptor_pool_);
}

DescriptorPool::operator vk::DescriptorPool() const
{
  return descriptor_pool_;
}
}
}
