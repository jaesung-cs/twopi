#include <twopi/vk/vk_buffer.h>

#include <twopi/vk/vk_device.h>
#include <twopi/vk/vk_device_memory.h>

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

Buffer::Creator& Buffer::Creator::SetVertexBufferSize(int size)
{
  create_info_
    .setSize(size)
    .setUsage(vk::BufferUsageFlagBits::eVertexBuffer);

  return *this;
}

Buffer Buffer::Creator::Create()
{
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
