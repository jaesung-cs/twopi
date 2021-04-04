#ifndef TWOPI_VKW_VKW_GRAPHICS_PIPELINE_H_
#define TWOPI_VKW_VKW_GRAPHICS_PIPELINE_H_

#include <initializer_list>

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Device;
class ShaderModule;
class PipelineCache;
class PipelineLayout;
class RenderPass;

class GraphicsPipeline
{
public:
  class Creator
  {
  private:
    struct Attribute
    {
      Attribute(int index, int size)
        : index(index), size(size) {}

      int index;
      int size;
    };

  public:
    Creator() = delete;
    Creator(Device device);
    ~Creator();

    Creator& SetPipelineCache(PipelineCache pipeline_cache);
    Creator& SetMultisample4();
    Creator& SetShader(ShaderModule vert_shader, ShaderModule frag_shader);
    Creator& SetVertexInput(std::initializer_list<Attribute> attributes);
    Creator& SetViewport(int width, int height);
    Creator& SetPipelineLayout(PipelineLayout pipeline_layout);
    Creator& SetRenderPass(RenderPass render_pass);

    GraphicsPipeline Create();

  private:
    const vk::Device device_;

    vk::GraphicsPipelineCreateInfo create_info_;
    vk::PipelineCache pipeline_cache_;
    std::vector<vk::PipelineShaderStageCreateInfo> shader_stages_;
    std::vector<vk::VertexInputBindingDescription> binding_descriptions_;
    std::vector<vk::VertexInputAttributeDescription> attribute_descriptions_;
    vk::PipelineVertexInputStateCreateInfo vertex_input_info_;
    vk::PipelineInputAssemblyStateCreateInfo input_assembly_info_;
    vk::Viewport viewport_;
    vk::Rect2D scissor_;
    vk::PipelineViewportStateCreateInfo viewport_state_info_;
    vk::PipelineRasterizationStateCreateInfo rasterizer_info_;
    vk::PipelineMultisampleStateCreateInfo multisample_info_;
    vk::PipelineColorBlendAttachmentState color_blend_attachment_;
    vk::PipelineColorBlendStateCreateInfo color_blend_info_;
    vk::PipelineDepthStencilStateCreateInfo depth_stencil_info_;
    std::vector<vk::DynamicState> dynamic_states_;
    vk::PipelineDynamicStateCreateInfo dynamic_state_info_;
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

#endif // TWOPI_VKW_VKW_GRAPHICS_PIPELINE_H_
