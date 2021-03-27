#include <twopi/vk/vk_graphics_pipeline.h>

#include <twopi/vk/vk_device.h>
#include <twopi/vk/vk_shader_module.h>
#include <twopi/vk/vk_pipeline_layout.h>
#include <twopi/vk/vk_render_pass.h>

namespace twopi
{
namespace vkw
{
//
// Creator
//
GraphicsPipeline::Creator::Creator(Device device)
  : device_(device)
{
  rasterizer_info_
    .setDepthClampEnable(false)
    .setRasterizerDiscardEnable(false)
    .setPolygonMode(vk::PolygonMode::eFill)
    .setLineWidth(1.f)
    .setCullMode(vk::CullModeFlagBits::eNone)
    .setFrontFace(vk::FrontFace::eCounterClockwise)
    .setDepthBiasEnable(false);

  multisample_info_
    .setSampleShadingEnable(false)
    .setRasterizationSamples(vk::SampleCountFlagBits::e1);

  color_blend_attachment_
    .setColorWriteMask(
      vk::ColorComponentFlagBits::eR |
      vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB |
      vk::ColorComponentFlagBits::eA)
    .setBlendEnable(false)
    .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
    .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
    .setColorBlendOp(vk::BlendOp::eAdd)
    .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
    .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
    .setAlphaBlendOp(vk::BlendOp::eAdd);

  color_blend_info_
    .setLogicOpEnable(false)
    .setAttachments(color_blend_attachment_)
    .setBlendConstants({ 0.f, 0.f, 0.f, 0.f });

  dynamic_states_ = { vk::DynamicState::eViewport, vk::DynamicState::eLineWidth };
  dynamic_state_info_
    .setDynamicStates(dynamic_states_);
}

GraphicsPipeline::Creator::~Creator() = default;

GraphicsPipeline::Creator& GraphicsPipeline::Creator::SetShader(ShaderModule vert_shader, ShaderModule frag_shader)
{
  shader_stages_.clear();

  vk::PipelineShaderStageCreateInfo vert_shader_stage_info{};
  vert_shader_stage_info
    .setStage(vk::ShaderStageFlagBits::eVertex)
    .setModule(vert_shader)
    .setPName("main");
  shader_stages_.push_back(vert_shader_stage_info);

  vk::PipelineShaderStageCreateInfo frag_shader_stage_info{};
  frag_shader_stage_info
    .setStage(vk::ShaderStageFlagBits::eFragment)
    .setModule(frag_shader)
    .setPName("main");
  shader_stages_.push_back(frag_shader_stage_info);

  return *this;
}

GraphicsPipeline::Creator& GraphicsPipeline::Creator::SetVertexInput(std::initializer_list<Attribute> attributes)
{
  for (const auto& attribute : attributes)
  {
    const auto& index = attribute.index;
    const auto& size = attribute.size;

    vk::Format format;
    switch (size)
    {
    case 2:
      format = vk::Format::eR32G32Sfloat;
      break;
    case 3:
      format = vk::Format::eR32G32B32Sfloat;
      break;
    }

    vk::VertexInputBindingDescription binding_description{};
    binding_description
      .setBinding(index)
      .setStride(size * sizeof(float))
      .setInputRate(vk::VertexInputRate::eVertex);
    binding_descriptions_.emplace_back(std::move(binding_description));

    vk::VertexInputAttributeDescription attribute_description{};
    attribute_description
      .setBinding(index)
      .setLocation(index)
      .setFormat(format)
      .setOffset(0);
    attribute_descriptions_.emplace_back(std::move(attribute_description));
  }

  // Set with binding and attribute descriptions
  vertex_input_info_
    .setVertexBindingDescriptions(binding_descriptions_)
    .setVertexAttributeDescriptions(attribute_descriptions_);

  input_assembly_info_
    .setTopology(vk::PrimitiveTopology::eTriangleList)
    .setPrimitiveRestartEnable(VK_FALSE);

  return *this;
}

GraphicsPipeline::Creator& GraphicsPipeline::Creator::SetViewport(int width, int height)
{
  viewport_
    .setX(0.f)
    .setY(0.f)
    .setWidth(static_cast<float>(width))
    .setHeight(static_cast<float>(height))
    .setMinDepth(0.f)
    .setMaxDepth(1.f);

  scissor_
    .setOffset({ 0, 0 })
    .setExtent({ static_cast<uint32_t>(width), static_cast<uint32_t>(height) });

  viewport_state_info_
    .setViewports(viewport_)
    .setScissors(scissor_);

  return *this;
}

GraphicsPipeline::Creator& GraphicsPipeline::Creator::SetPipelineLayout(PipelineLayout pipeline_layout)
{
  create_info_.setLayout(pipeline_layout);
  return *this;
}

GraphicsPipeline::Creator& GraphicsPipeline::Creator::SetRenderPass(RenderPass render_pass)
{
  create_info_
    .setRenderPass(render_pass)
    .setSubpass(0);

  return *this;
}

GraphicsPipeline GraphicsPipeline::Creator::Create()
{
  create_info_
    .setStages(shader_stages_)
    .setPVertexInputState(&vertex_input_info_)
    .setPInputAssemblyState(&input_assembly_info_)
    .setPViewportState(&viewport_state_info_)
    .setPRasterizationState(&rasterizer_info_)
    .setPMultisampleState(&multisample_info_)
    .setPDepthStencilState(nullptr)
    .setPColorBlendState(&color_blend_info_)
    .setPDynamicState(nullptr) // TODO
    .setBasePipelineHandle(nullptr)
    .setBasePipelineIndex(-1);

  auto create_result = device_.createGraphicsPipeline(nullptr, create_info_);
  // TODO: check result
  return GraphicsPipeline{ device_, create_result.value };
}

//
// GraphicsPipeline
//
GraphicsPipeline::GraphicsPipeline()
{
}

GraphicsPipeline::GraphicsPipeline(vk::Device device, vk::Pipeline pipeline)
  : device_(device), pipeline_(pipeline)
{
}

GraphicsPipeline::~GraphicsPipeline() = default;

void GraphicsPipeline::Destroy()
{
  device_.destroyPipeline(pipeline_);
}

GraphicsPipeline::operator vk::Pipeline() const
{
  return pipeline_;
}
}
}
