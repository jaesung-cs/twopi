#ifndef TWOPI_VK_VK_COMMAND_BUFFER_H_
#define TWOPI_VK_VK_COMMAND_BUFFER_H_

#include <vector>

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Device;
class CommandPool;
class RenderPass;
class Framebuffer;
class GraphicsPipeline;

class CommandBuffer
{
public:
  class Allocator
  {
  public:
    Allocator() = delete;
    explicit Allocator(Device device, CommandPool command_pool);
    ~Allocator();

    std::vector<CommandBuffer> Allocate(int size);

  private:
    const vk::Device device_;
    const vk::CommandPool command_pool_;

    vk::CommandBufferAllocateInfo allocate_info_{};
  };

public:
  CommandBuffer();
  CommandBuffer(vk::Device device, vk::CommandPool command_pool, vk::CommandBuffer command_buffer);

  ~CommandBuffer();

  void Free();

  CommandBuffer& Begin();
  CommandBuffer& BeginRenderPass(RenderPass render_pass, Framebuffer framebuffer);
  CommandBuffer& BindPipeline(GraphicsPipeline graphics_pipeline);
  CommandBuffer& Draw(int vertex_count, int instace_count, int first_vertex, int first_instance);
  CommandBuffer& EndRenderPass();
  void End();

  operator vk::CommandBuffer() const;

private:
  vk::Device device_;
  vk::CommandPool command_pool_;
  vk::CommandBuffer command_buffer_;
};
}
}

#endif // TWOPI_VK_VK_COMMAND_BUFFER_H_
