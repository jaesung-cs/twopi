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
  pool_size_.setType(vk::DescriptorType::eUniformBuffer);
}

DescriptorPool::Creator::~Creator() = default;

DescriptorPool::Creator& DescriptorPool::Creator::SetSize(uint32_t size)
{
  pool_size_.setDescriptorCount(size);
  return *this;
}

DescriptorPool DescriptorPool::Creator::Create()
{
  create_info_
    .setPoolSizes(pool_size_)
    .setMaxSets(pool_size_.descriptorCount);

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
