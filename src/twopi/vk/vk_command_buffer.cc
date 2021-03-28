#include <twopi/vk/vk_command_buffer.h>

#include <twopi/vk/vk_device.h>
#include <twopi/vk/vk_command_pool.h>
#include <twopi/vk/vk_render_pass.h>
#include <twopi/vk/vk_framebuffer.h>
#include <twopi/vk/vk_graphics_pipeline.h>
#include <twopi/vk/vk_buffer.h>

namespace twopi
{
namespace vkw
{
//
// Allocator
//
CommandBuffer::Allocator::Allocator(Device device, CommandPool command_pool)
  : device_(device), command_pool_(command_pool)
{
  allocate_info_
    .setCommandPool(command_pool)
    .setLevel(vk::CommandBufferLevel::ePrimary);
}

CommandBuffer::Allocator::~Allocator() = default;

std::vector<CommandBuffer> CommandBuffer::Allocator::Allocate(int size)
{
  allocate_info_.setCommandBufferCount(size);

  const auto command_buffer_handles = device_.allocateCommandBuffers(allocate_info_);
  std::vector<CommandBuffer> command_buffers;
  for (int i = 0; i < size; i++)
    command_buffers.emplace_back(CommandBuffer{ device_, command_pool_, command_buffer_handles[i] });
  return command_buffers;
}

//
// CommandBuffer
//
CommandBuffer::CommandBuffer()
{
}

CommandBuffer::CommandBuffer(vk::Device device, vk::CommandPool command_pool, vk::CommandBuffer command_buffer)
  : device_(device), command_pool_(command_pool), command_buffer_(command_buffer)
{
}

CommandBuffer::~CommandBuffer() = default;

void CommandBuffer::Free()
{
  device_.freeCommandBuffers(command_pool_, command_buffer_);
}

CommandBuffer& CommandBuffer::Begin()
{
  command_buffer_.begin(vk::CommandBufferBeginInfo{});
  return *this;
}

CommandBuffer& CommandBuffer::BeginRenderPass(RenderPass render_pass, Framebuffer framebuffer)
{
  vk::ClearValue clear_value(std::array<float, 4>{0.8f, 0.8f, 0.8f, 1.f});

  vk::RenderPassBeginInfo render_pass_begin_info;
  render_pass_begin_info
    .setRenderPass(render_pass)
    .setFramebuffer(framebuffer)
    .setRenderArea(vk::Rect2D({ 0, 0 }, { framebuffer.Width(), framebuffer.Height() }))
    .setClearValues(clear_value);

  command_buffer_.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

  return *this;
}

CommandBuffer& CommandBuffer::BindVertexBuffers(std::vector<Buffer> buffers, std::vector<uint64_t> offsets)
{
  std::vector<vk::Buffer> buffer_handles(buffers.cbegin(), buffers.cend());
  command_buffer_.bindVertexBuffers(0, buffer_handles, offsets);

  return *this;
}

CommandBuffer& CommandBuffer::BindIndexBuffer(Buffer buffer)
{
  command_buffer_.bindIndexBuffer(buffer, 0, vk::IndexType::eUint32);
  return *this;
}

CommandBuffer& CommandBuffer::BindPipeline(GraphicsPipeline graphics_pipeline)
{
  command_buffer_.bindPipeline(vk::PipelineBindPoint::eGraphics, graphics_pipeline);
  return *this;
}

CommandBuffer& CommandBuffer::Draw(int vertex_count, int instace_count, int first_vertex, int first_instance)
{
  command_buffer_.draw(vertex_count, instace_count, first_vertex, first_instance);
  return *this;
}

CommandBuffer& CommandBuffer::DrawIndexed(int index_count, int instance_count)
{
  command_buffer_.drawIndexed(index_count, instance_count, 0, 0, 0);
  return *this;
}

CommandBuffer& CommandBuffer::EndRenderPass()
{
  command_buffer_.endRenderPass();
  return *this;
}

CommandBuffer& CommandBuffer::BeginOneTime()
{
  vk::CommandBufferBeginInfo begin_info{};
  begin_info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

  command_buffer_.begin(begin_info);
  return *this;
}

CommandBuffer& CommandBuffer::CopyBuffer(Buffer src, Buffer dst, uint64_t size)
{
  return CopyBuffer(src, 0, dst, 0, size);
}

CommandBuffer& CommandBuffer::CopyBuffer(Buffer src, uint64_t src_offset, Buffer dst, uint64_t dst_offset, uint64_t size)
{
  vk::BufferCopy copy_region;
  copy_region
    .setSrcOffset(src_offset)
    .setDstOffset(dst_offset)
    .setSize(size);

  command_buffer_.copyBuffer(src, dst, copy_region);
  return *this;
}

void CommandBuffer::End()
{
  command_buffer_.end();
}

void CommandBuffer::Reset()
{
  command_buffer_.reset();
}

CommandBuffer::operator vk::CommandBuffer() const
{
  return command_buffer_;
}
}
}
