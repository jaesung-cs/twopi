#include <twopi/vkw/vkw_buffer.h>

#include <twopi/vkw/vkw_device.h>
#include <twopi/vkw/vkw_device_memory.h>

namespace twopi
{
namespace vkw
{
//
// Creator
//
Buffer::Creator::Creator(Device device)
  : device_(device)
{
  create_info_.setSharingMode(vk::SharingMode::eExclusive);
}

Buffer::Creator::~Creator() = default;

Buffer::Creator& Buffer::Creator::SetSize(uint64_t size)
{
  create_info_.setSize(size);
  return *this;
}

Buffer::Creator& Buffer::Creator::SetTransferSrcBuffer()
{
  usage_ |= vk::BufferUsageFlagBits::eTransferSrc;
  return *this;
}

Buffer::Creator& Buffer::Creator::SetTransferDstBuffer()
{
  usage_ |= vk::BufferUsageFlagBits::eTransferDst;
  return *this;
}

Buffer::Creator& Buffer::Creator::SetVertexBuffer()
{
  usage_ |= vk::BufferUsageFlagBits::eVertexBuffer;
  return *this;
}

Buffer::Creator& Buffer::Creator::SetIndexBuffer()
{
  usage_ |= vk::BufferUsageFlagBits::eIndexBuffer;
  return *this;
}

Buffer::Creator& Buffer::Creator::SetUniformBuffer()
{
  usage_ |= vk::BufferUsageFlagBits::eUniformBuffer;
  return *this;
}

Buffer Buffer::Creator::Create()
{
  create_info_.setUsage(usage_);
  const auto handle = device_.createBuffer(create_info_);
  return Buffer{ device_, handle };
}

//
// Buffer
//
Buffer::Buffer()
{
}

Buffer::Buffer(vk::Device device, vk::Buffer buffer)
  : device_(device), buffer_(buffer)
{
}

Buffer::~Buffer() = default;

void Buffer::Destroy()
{
  device_.destroyBuffer(buffer_);
}

Buffer::operator vk::Buffer() const
{
  return buffer_;
}

void Buffer::Bind(DeviceMemory memory)
{
  device_.bindBufferMemory(buffer_, memory, 0);
}
}
}
