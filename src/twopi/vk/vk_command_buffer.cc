#include <twopi/vk/vk_command_buffer.h>

#include <twopi/core/error.h>
#include <twopi/vk/vk_device.h>
#include <twopi/vk/vk_command_pool.h>
#include <twopi/vk/vk_render_pass.h>
#include <twopi/vk/vk_framebuffer.h>
#include <twopi/vk/vk_graphics_pipeline.h>
#include <twopi/vk/vk_image.h>
#include <twopi/vk/vk_buffer.h>
#include <twopi/vk/vk_pipeline_layout.h>
#include <twopi/vk/vk_descriptor_set.h>

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
  std::array<vk::ClearValue, 2> clear_values{
    vk::ClearColorValue{ std::array<float, 4>{0.8f, 0.8f, 0.8f, 1.f} },
    vk::ClearDepthStencilValue{ 1.f, 0u }
  };

  vk::RenderPassBeginInfo render_pass_begin_info;
  render_pass_begin_info
    .setRenderPass(render_pass)
    .setFramebuffer(framebuffer)
    .setRenderArea(vk::Rect2D({ 0, 0 }, { framebuffer.Width(), framebuffer.Height() }))
    .setClearValues(clear_values);

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

CommandBuffer& CommandBuffer::BindDescriptorSets(PipelineLayout layout, std::vector<DescriptorSet> descriptor_sets)
{
  std::vector<vk::DescriptorSet> descriptor_set_handles(descriptor_sets.cbegin(), descriptor_sets.cend());
  std::vector<uint32_t> offsets(descriptor_sets.size(), 0u);
  command_buffer_.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 0, descriptor_set_handles, nullptr);
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

CommandBuffer& CommandBuffer::PipelineBarrier(Image image, vk::ImageLayout old_layout, vk::ImageLayout new_layout, int mip_levels, int mip_level)
{
  vk::AccessFlags src_access_mask;
  vk::AccessFlags dst_access_mask;
  vk::PipelineStageFlags src_stage;
  vk::PipelineStageFlags dst_stage;

  if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal)
  {
    src_access_mask = vk::AccessFlagBits{};
    dst_access_mask = vk::AccessFlagBits::eTransferWrite;

    src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
    dst_stage = vk::PipelineStageFlagBits::eTransfer;
  }
  else if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
  {
    src_access_mask = vk::AccessFlagBits{};
    dst_access_mask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

    src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
    dst_stage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
  }
  else if (old_layout == vk::ImageLayout::eTransferSrcOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
  {
    src_access_mask = vk::AccessFlagBits::eTransferWrite;
    dst_access_mask = vk::AccessFlagBits::eShaderRead;

    src_stage = vk::PipelineStageFlagBits::eTransfer;
    dst_stage = vk::PipelineStageFlagBits::eFragmentShader;
  }
  else if (old_layout == vk::ImageLayout::eTransferDstOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
  {
    src_access_mask = vk::AccessFlagBits::eTransferWrite;
    dst_access_mask = vk::AccessFlagBits::eShaderRead;

    src_stage = vk::PipelineStageFlagBits::eTransfer;
    dst_stage = vk::PipelineStageFlagBits::eFragmentShader;
  }
  else if (old_layout == vk::ImageLayout::eTransferDstOptimal && new_layout == vk::ImageLayout::eTransferSrcOptimal)
  {
    src_access_mask = vk::AccessFlagBits::eTransferWrite;
    dst_access_mask = vk::AccessFlagBits::eTransferRead;

    src_stage = vk::PipelineStageFlagBits::eTransfer;
    dst_stage = vk::PipelineStageFlagBits::eTransfer;
  }
  else
    throw core::Error("Unsupported layout transition");

  vk::ImageAspectFlags aspect_mask = vk::ImageAspectFlagBits::eColor;
  if (new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
    aspect_mask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;

  vk::ImageMemoryBarrier barrier;
  barrier
    .setOldLayout(old_layout)
    .setNewLayout(new_layout)
    .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
    .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
    .setImage(image)
    .setSubresourceRange(vk::ImageSubresourceRange{}
      .setAspectMask(aspect_mask)
      .setBaseMipLevel(mip_level)
      .setLevelCount(mip_levels)
      .setBaseArrayLayer(0)
      .setLayerCount(1))
    .setSrcAccessMask(src_access_mask)
    .setDstAccessMask(dst_access_mask);

  command_buffer_.pipelineBarrier(
    src_stage,
    dst_stage,
    vk::DependencyFlagBits{},
    {},
    {},
    barrier
  );

  return *this;
}

CommandBuffer& CommandBuffer::CopyBuffer(Buffer src, Image dst)
{
  vk::BufferImageCopy region;
  region
    .setBufferOffset(0)
    .setBufferRowLength(0)
    .setBufferImageHeight(0)
    .setImageSubresource(vk::ImageSubresourceLayers{}
      .setAspectMask(vk::ImageAspectFlagBits::eColor)
      .setMipLevel(0)
      .setBaseArrayLayer(0)
      .setLayerCount(1))
    .setImageOffset(vk::Offset3D{ 0, 0, 0 })
    .setImageExtent(vk::Extent3D{ static_cast<uint32_t>(dst.Width()), static_cast<uint32_t>(dst.Height()), 1 });

  command_buffer_.copyBufferToImage(src, dst, vk::ImageLayout::eTransferDstOptimal, region);
  return *this;
}

CommandBuffer& CommandBuffer::BlitImage(Image image, uint32_t width, uint32_t height, int mip_level)
{
  std::array<vk::Offset3D, 2> src_offsets{ vk::Offset3D{ 0, 0, 0 }, vk::Offset3D{ static_cast<int>(width), static_cast<int>(height), 1 } };
  std::array<vk::Offset3D, 2> dst_offsets{ vk::Offset3D{ 0, 0, 0 }, vk::Offset3D{ static_cast<int>(width) / 2, static_cast<int>(height) / 2, 1 } };

  vk::ImageBlit region;
  region
    .setSrcOffsets(src_offsets)
    .setSrcSubresource(vk::ImageSubresourceLayers{ vk::ImageAspectFlagBits::eColor, static_cast<uint32_t>(mip_level) - 1, 0, 1 })
    .setDstOffsets(dst_offsets)
    .setDstSubresource(vk::ImageSubresourceLayers{ vk::ImageAspectFlagBits::eColor, static_cast<uint32_t>(mip_level), 0, 1 });

  command_buffer_.blitImage(
    image, vk::ImageLayout::eTransferSrcOptimal,
    image, vk::ImageLayout::eTransferDstOptimal,
    region, vk::Filter::eLinear);

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
