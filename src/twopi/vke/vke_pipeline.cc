#include <twopi/vke/vke_pipeline.h>

#include <fstream>

#include <twopi/core/error.h>
#include <twopi/vke/vke_context.h>
#include <twopi/vke/vke_swapchain.h>
#include <twopi/vke/vke_attribute.h>
#include <twopi/vke/vke_uniform.h>

namespace twopi
{
namespace vke
{
namespace
{
auto LoadCode(const std::string& filepath)
{
  std::vector<uint32_t> code;

  std::ifstream file(filepath, std::ios::ate | std::ios::binary);
  if (!file.is_open())
    throw core::Error("Failed to open file: " + filepath);

  size_t file_size = (size_t)file.tellg();
  std::vector<char> buffer(file_size);
  file.seekg(0);
  file.read(buffer.data(), file_size);
  file.close();

  auto* int_ptr = reinterpret_cast<uint32_t*>(buffer.data());
  for (int i = 0; i < file_size / 4; i++)
    code.push_back(int_ptr[i]);

  return code;
}
}

Pipeline::Pipeline(std::shared_ptr<vke::Context> context,
  std::shared_ptr<Swapchain> swapchain,
  const std::string& dirpath, const std::string& shader_name,
  const std::vector<Attribute>& attributes,
  const std::vector<Uniform>& uniforms)
  : context_(context)
{
  // Shader
  const auto vert_shader_filepath = dirpath + '/' + shader_name + ".vert.spv";
  const auto frag_shader_filepath = dirpath + '/' + shader_name + ".frag.spv";

  const auto vert_shader_code = LoadCode(vert_shader_filepath);
  const auto frag_shader_code = LoadCode(frag_shader_filepath);

  vk::ShaderModuleCreateInfo shader_module_create_info;
  shader_module_create_info.setCode(vert_shader_code);
  auto vert_shader_module = context->Device().createShaderModule(shader_module_create_info);

  shader_module_create_info.setCode(frag_shader_code);
  auto frag_shader_module = context->Device().createShaderModule(shader_module_create_info);

  vk::PipelineShaderStageCreateInfo vert_shader_stage_info;
  vert_shader_stage_info
    .setStage(vk::ShaderStageFlagBits::eVertex)
    .setModule(vert_shader_module)
    .setPName("main");

  vk::PipelineShaderStageCreateInfo frag_shader_stage_info;
  frag_shader_stage_info
    .setStage(vk::ShaderStageFlagBits::eFragment)
    .setModule(frag_shader_module)
    .setPName("main");

  std::vector<vk::PipelineShaderStageCreateInfo> shader_stages{ vert_shader_stage_info, frag_shader_stage_info };

  // Rasterization
  vk::PipelineRasterizationStateCreateInfo rasterization_info;
  rasterization_info
    .setDepthClampEnable(false)
    .setRasterizerDiscardEnable(false)
    .setPolygonMode(vk::PolygonMode::eFill)
    .setLineWidth(1.f)
    .setCullMode(vk::CullModeFlagBits::eNone)
    .setFrontFace(vk::FrontFace::eCounterClockwise)
    .setDepthBiasEnable(false);

  // Multisample
  vk::PipelineMultisampleStateCreateInfo multisample_info;
  multisample_info
    .setSampleShadingEnable(false)
    .setRasterizationSamples(vk::SampleCountFlagBits::e4);

  // Color blend
  vk::PipelineColorBlendAttachmentState color_blend_attachment;
  color_blend_attachment
    .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
    .setBlendEnable(false)
    .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
    .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
    .setColorBlendOp(vk::BlendOp::eAdd)
    .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
    .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
    .setAlphaBlendOp(vk::BlendOp::eAdd);

  vk::PipelineColorBlendStateCreateInfo color_blend_info;
  color_blend_info
    .setLogicOpEnable(false)
    .setAttachments(color_blend_attachment)
    .setBlendConstants({ 0.f, 0.f, 0.f, 0.f });

  // Depth stencil
  vk::PipelineDepthStencilStateCreateInfo depth_stencil_info;
  depth_stencil_info
    .setDepthTestEnable(true)
    .setDepthWriteEnable(true)
    .setDepthCompareOp(vk::CompareOp::eLess)
    .setDepthBoundsTestEnable(false)
    .setMinDepthBounds(0.f)
    .setMaxDepthBounds(1.f)
    .setStencilTestEnable(false)
    .setFront({})
    .setBack({});

  // Dynamic states
  std::vector<vk::DynamicState> dynamic_states = { vk::DynamicState::eViewport, vk::DynamicState::eLineWidth };
  vk::PipelineDynamicStateCreateInfo dynamic_state_info;
  dynamic_state_info
    .setDynamicStates(dynamic_states);

  // Viewport
  vk::Viewport viewport;
  viewport
    .setX(0.f)
    .setY(0.f)
    .setWidth(static_cast<float>(swapchain->Width()))
    .setHeight(static_cast<float>(swapchain->Height()))
    .setMinDepth(0.f)
    .setMaxDepth(1.f);

  vk::Rect2D scissor;
  scissor
    .setOffset({ 0, 0 })
    .setExtent({ swapchain->Width(), swapchain->Height() });

  vk::PipelineViewportStateCreateInfo viewport_info;
  viewport_info
    .setViewports(viewport)
    .setScissors(scissor);

  // TODO: Vertex input
  std::vector<vk::VertexInputBindingDescription> binding_descriptions;
  std::vector<vk::VertexInputAttributeDescription> attribute_description;
  vk::PipelineVertexInputStateCreateInfo vertex_input_info;

  // Primitive
  vk::PipelineInputAssemblyStateCreateInfo input_assembly_info;
  input_assembly_info
    .setTopology(vk::PrimitiveTopology::eTriangleList)
    .setPrimitiveRestartEnable(false);

  // TODO: Pipeline layout
  vk::PipelineLayout pipeline_layout;
  
  // Pipeline
  vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info;
  graphics_pipeline_create_info
    .setRenderPass(swapchain->RenderPass())
    .setSubpass(0)
    .setLayout(pipeline_layout)
    .setStages(shader_stages)
    .setPVertexInputState(&vertex_input_info)
    .setPInputAssemblyState(&input_assembly_info)
    .setPViewportState(&viewport_info)
    .setPRasterizationState(&rasterization_info)
    .setPMultisampleState(&multisample_info)
    .setPColorBlendState(&color_blend_info)
    .setPDepthStencilState(&depth_stencil_info)
    .setPDynamicState(&dynamic_state_info)
    .setBasePipelineHandle(nullptr)
    .setBasePipelineIndex(-1);

  context->Device().createGraphicsPipeline(context->PipelineCache(), graphics_pipeline_create_info);

  context->Device().destroyShaderModule(vert_shader_module);
  context->Device().destroyShaderModule(frag_shader_module);
}

Pipeline::~Pipeline()
{
  Context()->Device().destroyPipeline(graphics_pipeline_);
  Context()->Device().destroyPipelineLayout(pipeline_layout_);
}

std::shared_ptr<Context> Pipeline::Context() const
{
  return context_.lock();
}
}
}
