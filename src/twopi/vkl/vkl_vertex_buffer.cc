#include <twopi/vkl/vkl_vertex_buffer.h>

#include <cstring>
#include <algorithm>

#include <twopi/vkl/vkl_context.h>

namespace twopi
{
namespace vkl
{
namespace
{
vk::DeviceSize Align(vk::DeviceSize offset, vk::DeviceSize align)
{
  return (offset + align - 1) & ~(align - 1ull);
}
}

VertexBuffer::VertexBuffer(std::shared_ptr<vkl::Context> context, int num_vertices, int num_indices)
  : Object{ context }
  , num_vertices_{ num_vertices }
  , num_indices_{ num_indices }
{
}

VertexBuffer::~VertexBuffer()
{
  const auto device = Context()->Device();

  device.destroyBuffer(buffer_);
}

void VertexBuffer::Prepare()
{
  const auto device = Context()->Device();

  std::sort(attributes_.begin(), attributes_.end(), [](const Attribute& lhs, const Attribute& rhs) {
    return lhs.index < rhs.index;
    });

  offsets_.push_back(0);
  sizes_.push_back(attributes_[0].byte_size * attributes_[0].size * num_vertices_);
  for (int i = 1; i < attributes_.size(); i++)
  {
    const auto offset = Align(offsets_[i - 1] + sizes_[i - 1], attributes_[i].byte_size);
    offsets_.push_back(offset);
    sizes_.push_back(attributes_[i].byte_size * attributes_[i].size * num_vertices_);
  }

  index_offset_ = Align(offsets_.back() + sizes_.back(), sizeof(uint32_t));
  index_size_ = sizeof(uint32_t) * num_indices_;

  buffer_size_ = index_size_;
  for (auto size : sizes_)
    buffer_size_ += size;

  vk::BufferCreateInfo buffer_create_info;
  buffer_create_info
    .setSharingMode(vk::SharingMode::eExclusive)
    .setUsage(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer)
    .setSize(buffer_size_);
  buffer_ = device.createBuffer(buffer_create_info);

  memory_ = Context()->AllocateDeviceMemory(buffer_);
  device.bindBufferMemory(buffer_, memory_.device_memory, memory_.offset);
}

vk::DeviceSize VertexBuffer::IndexOffset() const
{
  return index_offset_;
}

vk::DeviceSize VertexBuffer::IndexSize() const
{
  return index_size_;
}

vk::DeviceSize VertexBuffer::Offset(int index) const
{
  return offsets_[index];
}

vk::DeviceSize VertexBuffer::Size(int index) const
{
  return sizes_[index];
}
}
}
