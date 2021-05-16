#include <twopi/vkl/model/vkl_cubeskin.h>

namespace twopi
{
namespace vkl
{
Cubeskin::Cubeskin(std::shared_ptr<vkl::Context> context, int segments, int depth)
  : Object(context)
  , segments_(segments)
  , depth_(depth)
{
  std::vector<float> vertex_buffer;
  for (int i = 0; i < segments; i++)
  {
    const auto u = static_cast<float>(i) / (segments - 1);
    const auto x = u * 2.f - 1.f;
    for (int j = 0; j < segments; j++)
    {
      const auto v = static_cast<float>(j) / (segments - 1);
      const auto y = v * 2.f - 1.f;
      for (int k = 0; k < depth; k++)
      {
        const auto z = static_cast<float>(k) / depth;

        vertex_buffer.push_back(x);
        vertex_buffer.push_back(y);
        vertex_buffer.push_back(z);
        vertex_buffer.push_back(0.f);
        vertex_buffer.push_back(0.f);
        vertex_buffer.push_back(0.f);
      }
    }
  }

  std::vector<uint32_t> support_index_buffer;
  const auto index = [&segments](int i, int j, int k) {
    return (k * segments + i) * segments + j;
  };

  for (int k = 0; k < depth; k++)
  {
    for (int i = 0; i < segments; i++)
    {
      for (int j = 0; j < segments; j++)
        support_index_buffer.push_back(index(i, j, k));
      support_index_buffer.push_back(-1);
    }
    for (int j = 0; j < segments; j++)
    {
      for (int i = 0; i < segments; i++)
        support_index_buffer.push_back(index(i, j, k));
      support_index_buffer.push_back(-1);
    }
  }
  for (int i = 0; i < segments; i++)
  {
    for (int j = 0; j < segments; j++)
    {
      for (int k=0; k<depth; k++)
        support_index_buffer.push_back(index(i, j, k));
    }
    support_index_buffer.push_back(-1);
  }

  // Delete last (-1) index
  support_index_buffer.pop_back();

  // Allocate gpu buffer/memory
  const auto device = context->Device();
  vk::BufferCreateInfo buffer_create_info;
  buffer_create_info
    .setSharingMode(vk::SharingMode::eExclusive)
    .setUsage(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer)
    .setSize(vertex_buffer.size() * sizeof(float) * 2); // Double buffer
  shell_buffer_ = device.createBuffer(buffer_create_info);
  shell_memory_ = context->AllocateDeviceMemory(shell_buffer_);

  buffer_create_info
    .setUsage(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer)
    .setSize(support_index_buffer.size() * sizeof(uint32_t));
  index_buffer_ = device.createBuffer(buffer_create_info);
  index_memory_ = context->AllocateDeviceMemory(index_buffer_);

  context->ToGpu(vertex_buffer, shell_buffer_, 0);
  context->ToGpu(support_index_buffer, index_buffer_, 0);
}

Cubeskin::~Cubeskin()
{
  const auto device = Context()->Device();

  device.destroyBuffer(shell_buffer_);
  device.destroyBuffer(index_buffer_);
}

void Cubeskin::Update(vk::CommandBuffer& command_buffer)
{
}

void Cubeskin::DrawSupports(vk::CommandBuffer& command_buffer)
{
  // Line strip
}
}
}
