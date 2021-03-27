#ifndef TWOPI_VK_VK_GRAPHICS_PIPELINE_H_
#define TWOPI_VK_VK_GRAPHICS_PIPELINE_H_

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Device;
class ShaderModule;
class PipelineLayout;
class RenderPass;

class GraphicsPipeline
{
public:
  class Creator
  {
  public:
    Creator() = delete;
    Creator(Device device);
    ~Creator();

    Creator& SetShader(ShaderModule vert_shader, ShaderModule frag_shader);
    Creator& SetVertexInput();
    Creator& SetViewport(int width, int height);
    Creator& SetPipelineLayout(PipelineLayout pipeline_layout);
    Creator& SetRenderPass(RenderPass render_pass);

    GraphicsPipeline Create();

  private:
    const vk::Device device_;

    vk::GraphicsPipelineCreateInfo create_info_{};
    std::vector<vk::PipelineShaderStageCreateInfo> shader_stages_;
    vk::PipelineVertexInputStateCreateInfo vertex_input_info_{};
    vk::PipelineInputAssemblyStateCreateInfo input_assembly_info_{};
    vk::Viewport viewport_{};
    vk::Rect2D scissor_{};
    vk::PipelineViewportStateCreateInfo viewport_state_info_{};
    vk::PipelineRasterizationStateCreateInfo rasterizer_info_{};
    vk::PipelineMultisampleStateCreateInfo multisample_info_{};
    vk::PipelineColorBlendAttachmentState color_blend_attachment_{};
    vk::PipelineColorBlendStateCreateInfo color_blend_info_{};
    std::vector<vk::DynamicState> dynamic_states_;
    vk::PipelineDynamicStateCreateInfo dynamic_state_info_{};
  };

public:
  GraphicsPipeline();
  GraphicsPipeline(vk::Device device, vk::Pipeline pipeline);

  ~GraphicsPipeline();
  
  void Destroy();

  operator vk::Pipeline() const;

private:
  vk::Device device_;
  vk::Pipeline pipeline_;
};
}
}

#endif // TWOPI_VK_VK_GRAPHICS_PIPELINE_H_
