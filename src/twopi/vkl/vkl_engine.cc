#include <twopi/vkl/vkl_engine.h>

#include <iostream>
#include <fstream>
#include <optional>

#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <twopi/vkl/vkl_context.h>
#include <twopi/vkl/vkl_rendertarget.h>
#include <twopi/vkl/vkl_swapchain.h>
#include <twopi/vkl/vkl_uniform_buffer.h>
#include <twopi/vkl/vkl_vertex_buffer.h>
#include <twopi/vkl/model/vkl_cubeskin.h>
#include <twopi/vkl/primitive/vkl_sphere.h>
#include <twopi/vkl/primitive/vkl_floor.h>
#include <twopi/vkl/primitive/vkl_surface.h>
#include <twopi/core/error.h>
#include <twopi/window/window.h>
#include <twopi/window/glfw_window.h>
#include <twopi/scene/camera.h>
#include <twopi/scene/light.h>
#include <twopi/geometry/mesh.h>
#include <twopi/geometry/mesh_loader.h>

namespace twopi
{
namespace vkl
{
class Engine::Impl
{
private:
  enum class DrawMode
  {
    SOLID,
    WIREFRAME,
  };

  struct Buffer
  {
    vk::Buffer buffer;
    Memory memory;
  };

  // Binding 0
  struct CameraUbo
  {
    alignas(16) glm::mat4 projection;
    alignas(16) glm::mat4 view;
    alignas(16) glm::vec3 eye;
  };

  // Binding 1
  struct ModelUbo
  {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat3x4 model_inverse_transpose;
  };

  // Binding 2
  struct LightUbo
  {
    struct Light
    {
      alignas(16) glm::vec3 position;
      alignas(16) glm::vec3 ambient;
      alignas(16) glm::vec3 diffuse;
      alignas(16) glm::vec3 specular;
    };

    static constexpr int max_num_lights = 8;
    Light directional_lights[max_num_lights];
    Light point_lights[max_num_lights];
  };

  // Binding 3
  struct MaterialUbo
  {
    alignas(16) glm::vec3 specular;
    float shininess; // Padded
  };

public:
  Impl() = delete;

  Impl(std::shared_ptr<window::Window> window)
  {
    glfw_window_handle_ = std::dynamic_pointer_cast<window::GlfwWindow>(window)->Handle();

    width_ = window->Width();
    height_ = window->Height();
    max_width_ = window->MaxWidth();
    max_height_ = window->MaxHeight();

    mip_levels_ = 3;

    num_objects_ = 3; // One floor, one light, one cubeskin

    material_.specular = glm::vec3(1.f, 1.f, 1.f);
    material_.shininess = 64.f;

    floor_model_.model = glm::mat4(1.f);
    floor_model_.model_inverse_transpose = glm::inverse(glm::transpose(floor_model_.model));

    surface_model_.model = glm::mat4(1.f);
    surface_model_.model[3][2] = 3.f;
    surface_model_.model_inverse_transpose = glm::inverse(glm::transpose(surface_model_.model));

    cubeskin_model_.model = glm::mat4(1.f);
    cubeskin_model_.model[3][2] = 1.f;
    cubeskin_model_.model_inverse_transpose = glm::inverse(glm::transpose(cubeskin_model_.model));

    Prepare();
  }

  ~Impl()
  {
    Cleanup();
  }

  void Draw(core::Duration duration)
  {
    // Can be invisible, when window is minimized for example
    if (!visible_)
      return;

    const auto device = context_->Device();
    const auto queue = context_->Queue();
    const auto present_queue = context_->PresentQueue();

    auto wait_result = device.waitForFences(in_flight_fences_[current_frame_], true, UINT64_MAX);

    const auto result = device.acquireNextImageKHR(*swapchain_, UINT64_MAX, image_available_semaphores_[current_frame_]);
    if (result.result == vk::Result::eErrorOutOfDateKHR)
    {
      RecreateSwapchain();
      return;
    }
    else if (result.result != vk::Result::eSuccess && result.result != vk::Result::eSuboptimalKHR)
      throw core::Error("Failed to acquire next swapchain image.");

    const auto& image_index = result.value;

    if (images_in_flight_[image_index])
      wait_result = device.waitForFences(images_in_flight_[image_index], true, UINT64_MAX);
    images_in_flight_[image_index] = in_flight_fences_[current_frame_];

    device.resetFences(in_flight_fences_[current_frame_]);

    // Rebuild command buffers
    auto& command_buffer = draw_command_buffers_[image_index];
    BuildDrawCommandBuffer(command_buffer, image_index);

    // Update uniforms
    camera_ubos_[image_index] = camera_;
    light_ubos_[image_index] = lights_;
    model_ubos_[image_index][0] = floor_model_;
    model_ubos_[image_index][1] = light_model_;
    model_ubos_[image_index][2] = surface_model_;
    model_ubos_[image_index][3] = cubeskin_model_;
    material_ubos_[image_index] = material_;

    // Submit to graphics queue
    command_buffer.end();

    std::vector<vk::PipelineStageFlags> stage_mask = {
      vk::PipelineStageFlagBits::eColorAttachmentOutput,
    };
    vk::SubmitInfo submit_info;
    submit_info
      .setWaitSemaphores(image_available_semaphores_[current_frame_])
      .setCommandBuffers(command_buffer)
      .setSignalSemaphores(render_finished_semaphores_[current_frame_])
      .setWaitDstStageMask(stage_mask);
    queue.submit(submit_info, in_flight_fences_[current_frame_]);

    // Submit to present queue
    std::vector<uint32_t> image_indices{ image_index };
    vk::PresentInfoKHR present_info;
    std::vector<vk::SwapchainKHR> swapchains = { *swapchain_ };
    present_info
      .setSwapchains(swapchains)
      .setWaitSemaphores(render_finished_semaphores_[current_frame_])
      .setImageIndices(image_indices);
    const auto present_result = present_queue.presentKHR(present_info);

    if (present_result == vk::Result::eErrorOutOfDateKHR || present_result == vk::Result::eSuboptimalKHR)
      RecreateSwapchain();
    else if (present_result != vk::Result::eSuccess)
      throw core::Error("Failed to present swapchain image.");

    current_frame_ = (current_frame_ + 1) % max_frames_in_flight_;
  }

  void Resize(int width, int height)
  {
    std::cout << "Resized: " << width << ' ' << height << std::endl;

    width_ = width;
    height_ = height;

    if (width_ <= 0 || height_ <= 0)
      visible_ = false;
    else
    {
      visible_ = true;
      RecreateSwapchain();
    }
  }

  void UpdateLights(const std::vector<std::shared_ptr<scene::Light>>& lights)
  {
    int num_directional_lights = 0;
    int num_point_lights = 0;

    for (auto light : lights)
    {
      LightUbo::Light light_data;
      light_data.position = light->Position();
      light_data.ambient = light->Ambient();
      light_data.diffuse = light->Diffuse();
      light_data.specular = light->Specular();

      if (light->IsDirectionalLight())
      {
        light_data.position = glm::normalize(light_data.position);
        lights_.directional_lights[num_directional_lights++] = light_data;
      }
      else if (light->IsPointLight())
      {
        lights_.point_lights[num_point_lights++] = light_data;

        light_model_.model = glm::mat4(0.2f);
        light_model_.model[3] = glm::vec4(light->Position(), 1.f);
        light_model_.model_inverse_transpose = glm::inverse(glm::transpose(light_model_.model));
      }
    }
  }

  void UpdateCamera(std::shared_ptr<scene::Camera> camera)
  {
    camera_.projection = camera->ProjectionMatrix();
    camera_.projection[1][1] *= -1.f;

    camera_.view = camera->ViewMatrix();

    camera_.eye = camera->Eye();
  }

  void SetDrawSolid()
  {
    draw_mode_ = DrawMode::SOLID;
  }

  void SetDrawWireframe()
  {
    draw_mode_ = DrawMode::WIREFRAME;
  }

  void SetDrawNormal(bool draw_normal)
  {
    draw_normal_ = draw_normal;
  }

private:
  void BuildDrawCommandBuffer(vk::CommandBuffer& command_buffer, int image_index)
  {
    constexpr float line_width = 1.f;

    command_buffer.reset();

    vk::CommandBufferBeginInfo begin_info;
    command_buffer.begin(begin_info);

    std::array<vk::ClearValue, 2> clear_values = {
      vk::ClearColorValue{ std::array<float, 4>{ 0.8f, 0.8f, 0.8f, 1.f } },
      vk::ClearDepthStencilValue{ 1.f, 0u }
    };

    vk::Viewport viewport{ 0.f, 0.f, static_cast<float>(width_), static_cast<float>(height_), 0.f, 1.f };
    command_buffer.setViewport(0, viewport);

    vk::Rect2D scissor{ { 0, 0 }, { width_, height_ } };
    command_buffer.setScissor(0, scissor);

    command_buffer.setLineWidth(line_width);

    vk::RenderPassBeginInfo render_pass_begin_info;
    render_pass_begin_info
      .setClearValues(clear_values)
      .setRenderPass(render_pass_)
      .setFramebuffer(framebuffers_[image_index])
      .setRenderArea(vk::Rect2D{ {0, 0}, {width_, height_} });
    command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

    // Sphere
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, color_pipeline_);

    command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout_, 0,
      { descriptor_sets_[image_index] }, { model_ubos_[image_index].Stride(), 0 });

    command_buffer.bindVertexBuffers(0,
      { sphere_vbo_->Buffer(), sphere_vbo_->Buffer() },
      { sphere_vbo_->Offset(0), sphere_vbo_->Offset(1) });

    command_buffer.bindIndexBuffer(sphere_vbo_->Buffer(), sphere_vbo_->IndexOffset(), vk::IndexType::eUint32);

    command_buffer.drawIndexed(sphere_vbo_->NumIndices(), 1, 0, 0, 0);

    // Floor
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, floor_pipeline_);

    command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout_, 0,
      { descriptor_sets_[image_index] }, { 0ull, 0ull });

    command_buffer.bindVertexBuffers(0,
      { floor_vbo_->Buffer(), floor_vbo_->Buffer(), floor_vbo_->Buffer() },
      { floor_vbo_->Offset(0), floor_vbo_->Offset(1), floor_vbo_->Offset(2) });

    command_buffer.bindIndexBuffer(floor_vbo_->Buffer(), floor_vbo_->IndexOffset(), vk::IndexType::eUint32);

    command_buffer.drawIndexed(floor_vbo_->NumIndices(), 1, 0, 0, 0);

    // Surface
    switch (draw_mode_)
    {
    case DrawMode::WIREFRAME:
      command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, surface_tessellation_wireframe_pipeline_);
      break;
    case DrawMode::SOLID:
    default:
      command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, surface_tessellation_pipeline_);
      break;
    }

    command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout_, 0,
      { descriptor_sets_[image_index] }, { model_ubos_[image_index].Stride() * 2, 0ull });

    command_buffer.bindVertexBuffers(0,
      { surface_vbo_->Buffer(), surface_vbo_->Buffer(), surface_vbo_->Buffer() },
      { surface_vbo_->Offset(0), surface_vbo_->Offset(1), surface_vbo_->Offset(2) });

    command_buffer.bindIndexBuffer(surface_vbo_->Buffer(), surface_vbo_->IndexOffset(), vk::IndexType::eUint32);

    command_buffer.drawIndexed(surface_vbo_->NumIndices(), 1, 0, 0, 0);

    // Surface normal
    if (draw_normal_)
    {
      command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, surface_tessellation_normal_pipeline_);

      command_buffer.drawIndexed(surface_vbo_->NumIndices(), 1, 0, 0, 0);
    }

    // Cubeskin support lines
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, cubeskin_support_lines_pipeline_);

    command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout_, 0,
      { descriptor_sets_[image_index] }, { model_ubos_[image_index].Stride() * 3, 0ull });

    cubeskin_->DrawSupports(command_buffer);

    command_buffer.endRenderPass();
  }

  void Prepare()
  {
    context_ = std::make_shared<vkl::Context>(glfw_window_handle_);

    std::cout << "Creating swapchain" << std::endl;
    CreateSwapchain();
    std::cout << "Creating render pass" << std::endl;
    CreateRenderPass();
    std::cout << "Creating swapchain framebuffers" << std::endl;
    CreateSwapchainFramebuffers();
    std::cout << "Creating sampler" << std::endl;
    CreateSampler();
    std::cout << "Creating descriptor set" << std::endl;
    CreateDescriptorSet();
    std::cout << "Creating pipelines" << std::endl;
    CreateGraphicsPipelines();
    std::cout << "Creating synchronization objects" << std::endl;
    CreateSynchronizationObjects();
    std::cout << "Preparing descriptors" << std::endl;
    PrepareDescriptors();
    std::cout << "Preparing resources" << std::endl;
    PrepareResources();
    std::cout << "Allocating draw command buffers" << std::endl;
    AllocateDrawCommandBuffers();

    std::cout << "Vulkan engine is ready" << std::endl;
  }

  void Cleanup()
  {
    const auto device = context_->Device();

    device.waitIdle();

    FreeDrawCommandBuffers();
    CleanupResources();
    DestroySynchronizationObjects();
    DestroyGraphicsPipelines();
    DestroyDesciptorSet();
    DestroySampler();
    DestroySwapchainFramebuffers();
    DestroyRenderPass();
    DestroySwapchain();
    context_.reset();
  }

  void RecreateSwapchain()
  {
    const auto device = context_->Device();

    device.waitIdle();

    DestroySwapchainFramebuffers();
    DestroyRenderPass();
    DestroySwapchain();

    CreateSwapchain();
    CreateRenderPass();
    CreateSwapchainFramebuffers();
  }

  void CreateSwapchain()
  {
    swapchain_ = std::make_unique<Swapchain>(context_, width_, height_);
  }

  void DestroySwapchain()
  {
    swapchain_.reset();
  }

  void CreateRenderPass()
  {
    const auto device = context_->Device();

    constexpr auto samples = vk::SampleCountFlagBits::e4;

    rendertarget_ = std::make_unique<Rendertarget>(context_, max_width_, max_height_, width_, height_, swapchain_->ImageFormat(), samples);

    // Create render pass
    vk::AttachmentDescription color_attachment;
    color_attachment
      .setSamples(samples)
      .setLoadOp(vk::AttachmentLoadOp::eClear)
      .setStoreOp(vk::AttachmentStoreOp::eStore)
      .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
      .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
      .setInitialLayout(vk::ImageLayout::eUndefined)
      .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal)
      .setFormat(swapchain_->ImageFormat());

    vk::AttachmentReference color_attachment_ref;
    color_attachment_ref
      .setAttachment(0)
      .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::AttachmentDescription depth_attachment;
    depth_attachment
      .setSamples(samples)
      .setLoadOp(vk::AttachmentLoadOp::eClear)
      .setStoreOp(vk::AttachmentStoreOp::eDontCare)
      .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
      .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
      .setInitialLayout(vk::ImageLayout::eUndefined)
      .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
      .setFormat(vk::Format::eD24UnormS8Uint);

    vk::AttachmentReference depth_attachment_ref;
    depth_attachment_ref
      .setAttachment(1)
      .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::AttachmentDescription color_resolve_attachment;
    color_resolve_attachment
      .setSamples(vk::SampleCountFlagBits::e1)
      .setLoadOp(vk::AttachmentLoadOp::eDontCare)
      .setStoreOp(vk::AttachmentStoreOp::eStore)
      .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
      .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
      .setInitialLayout(vk::ImageLayout::eUndefined)
      .setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
      .setFormat(swapchain_->ImageFormat());

    vk::AttachmentReference color_resolve_attachment_ref;
    color_resolve_attachment_ref
      .setAttachment(2)
      .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpass;
    subpass
      .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
      .setColorAttachments(color_attachment_ref)
      .setPDepthStencilAttachment(&depth_attachment_ref)
      .setResolveAttachments(color_resolve_attachment_ref);

    vk::SubpassDependency dependency;
    dependency
      .setSrcSubpass(VK_SUBPASS_EXTERNAL)
      .setDstSubpass(0)
      .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
      .setSrcAccessMask(vk::AccessFlags{})
      .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
      .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

    const std::vector<vk::AttachmentDescription> attachments = {
      color_attachment,
      depth_attachment,
      color_resolve_attachment,
    };

    vk::RenderPassCreateInfo render_pass_create_info;
    render_pass_create_info
      .setAttachments(attachments)
      .setSubpasses(subpass)
      .setDependencies(dependency);

    render_pass_ = device.createRenderPass(render_pass_create_info);
  }

  void DestroyRenderPass()
  {
    const auto device = context_->Device();

    device.destroyRenderPass(render_pass_);

    rendertarget_.reset();
  }

  void CreateSwapchainFramebuffers()
  {
    const auto device = context_->Device();

    // Create framebuffers
    vk::FramebufferCreateInfo framebuffer_create_info;
    framebuffer_create_info
      .setWidth(width_)
      .setHeight(height_)
      .setLayers(1)
      .setRenderPass(render_pass_);

    framebuffers_.resize(swapchain_->Images().size());
    for (uint32_t i = 0; i < swapchain_->ImageCount(); i++)
    {
      const std::vector<vk::ImageView> attachments = {
        rendertarget_->ColorImageView(),
        rendertarget_->DepthImageView(),
        swapchain_->ImageViews()[i],
      };

      framebuffer_create_info
        .setAttachments(attachments);

      framebuffers_[i] = device.createFramebuffer(framebuffer_create_info);
    }
  }

  void DestroySwapchainFramebuffers()
  {
    const auto device = context_->Device();

    for (auto& framebuffer : framebuffers_)
      device.destroyFramebuffer(framebuffer);
    framebuffers_.clear();
  }

  void CreateSampler()
  {
    const auto physical_device = context_->PhysicalDevice();
    const auto device = context_->Device();

    vk::SamplerCreateInfo sampler_create_info;
    sampler_create_info
      .setAnisotropyEnable(true)
      .setMaxAnisotropy(physical_device.getProperties().limits.maxSamplerAnisotropy)
      .setMagFilter(vk::Filter::eLinear)
      .setMinFilter(vk::Filter::eLinear)
      .setAddressModeU(vk::SamplerAddressMode::eRepeat)
      .setAddressModeV(vk::SamplerAddressMode::eRepeat)
      .setAddressModeW(vk::SamplerAddressMode::eRepeat)
      .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
      .setUnnormalizedCoordinates(false)
      .setCompareEnable(false)
      .setCompareOp(vk::CompareOp::eAlways)
      .setMipmapMode(vk::SamplerMipmapMode::eLinear)
      .setMipLodBias(0.f)
      .setMinLod(0.f)
      .setMaxLod(static_cast<float>(mip_levels_));

    sampler_ = device.createSampler(sampler_create_info);
  }

  void DestroySampler()
  {
    const auto device = context_->Device();

    device.destroySampler(sampler_);
  }

  void CreateDescriptorSet()
  {
    const auto device = context_->Device();

    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    vk::DescriptorSetLayoutBinding binding;

    // Binding 0: CameraUbo
    binding
      .setBinding(0)
      .setDescriptorType(vk::DescriptorType::eUniformBuffer)
      .setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eTessellationEvaluation | vk::ShaderStageFlagBits::eGeometry | vk::ShaderStageFlagBits::eFragment)
      .setDescriptorCount(1);
    bindings.push_back(binding);
    
    // Binding 1: ModelUbo
    binding
      .setBinding(1)
      .setDescriptorType(vk::DescriptorType::eUniformBufferDynamic)
      .setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
      .setDescriptorCount(1);
    bindings.push_back(binding);

    // Binding 2: LightUbo
    binding
      .setBinding(2)
      .setDescriptorType(vk::DescriptorType::eUniformBuffer)
      .setStageFlags(vk::ShaderStageFlagBits::eFragment)
      .setDescriptorCount(1);
    bindings.push_back(binding);
    
    // Binding 3: MaterialUbo
    binding
      .setBinding(3)
      .setDescriptorType(vk::DescriptorType::eUniformBufferDynamic)
      .setStageFlags(vk::ShaderStageFlagBits::eFragment)
      .setDescriptorCount(1);
    bindings.push_back(binding);

    // Binding 4: Sampler
    binding
      .setBinding(4)
      .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
      .setStageFlags(vk::ShaderStageFlagBits::eFragment)
      .setDescriptorCount(1);
    bindings.push_back(binding);

    vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_create_info;
    descriptor_set_layout_create_info
      .setBindings(bindings);

    descriptor_set_layout_ = device.createDescriptorSetLayout(descriptor_set_layout_create_info);

    // Descriptor pool
    std::vector<vk::DescriptorPoolSize> pool_sizes;
    vk::DescriptorPoolSize pool_size;

    // TODO: set appropriate descriptor count
    constexpr int descriptor_count = 100;

    pool_size
      .setType(vk::DescriptorType::eUniformBuffer)
      .setDescriptorCount(descriptor_count * 2);
    pool_sizes.push_back(pool_size);

    pool_size
      .setType(vk::DescriptorType::eUniformBufferDynamic)
      .setDescriptorCount(descriptor_count * 2);
    pool_sizes.push_back(pool_size);

    pool_size
      .setType(vk::DescriptorType::eCombinedImageSampler)
      .setDescriptorCount(descriptor_count);
    pool_sizes.push_back(pool_size);

    vk::DescriptorPoolCreateInfo descriptor_pool_create_info;
    descriptor_pool_create_info
      .setMaxSets(descriptor_count)
      .setPoolSizes(pool_sizes);
    descriptor_pool_ = device.createDescriptorPool(descriptor_pool_create_info);
  }

  void DestroyDesciptorSet()
  {
    const auto device = context_->Device();

    device.destroyDescriptorPool(descriptor_pool_);
    device.destroyDescriptorSetLayout(descriptor_set_layout_);
  }

  void CreateGraphicsPipelines()
  {
    const auto device = context_->Device();

    vk::PipelineRasterizationStateCreateInfo rasterization_info;
    rasterization_info
      .setDepthClampEnable(false)
      .setRasterizerDiscardEnable(false)
      .setPolygonMode(vk::PolygonMode::eFill)
      .setLineWidth(1.f)
      .setCullMode(vk::CullModeFlagBits::eBack)
      .setFrontFace(vk::FrontFace::eCounterClockwise)
      .setDepthBiasEnable(false);

    vk::PipelineMultisampleStateCreateInfo multisample_info;
    multisample_info
      .setSampleShadingEnable(false)
      .setRasterizationSamples(vk::SampleCountFlagBits::e4);

    vk::PipelineColorBlendAttachmentState color_blend_attachment;
    color_blend_attachment
      .setColorWriteMask(
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA)
      .setBlendEnable(true)
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

    // Viewport
    vk::Viewport viewport;
    viewport
      .setX(0.f).setY(0.f)
      .setWidth(static_cast<float>(width_))
      .setHeight(static_cast<float>(height_))
      .setMinDepth(0.f)
      .setMaxDepth(1.f);

    vk::Rect2D scissor;
    scissor
      .setOffset({ 0, 0 })
      .setExtent({ width_, height_ });

    vk::PipelineViewportStateCreateInfo viewport_state_info;
    viewport_state_info
      .setViewports(viewport)
      .setScissors(scissor);

    std::vector<vk::DynamicState> dynamic_states = {
      vk::DynamicState::eViewport,
      vk::DynamicState::eScissor,
      vk::DynamicState::eLineWidth,
    };

    vk::PipelineDynamicStateCreateInfo dynamic_state_info;
    dynamic_state_info
      .setDynamicStates(dynamic_states);

    vk::PipelineInputAssemblyStateCreateInfo input_assembly_info;
    input_assembly_info
      .setTopology(vk::PrimitiveTopology::eTriangleList)
      .setPrimitiveRestartEnable(false);

    // Pipeline layout
    vk::PipelineLayoutCreateInfo pipeline_layout_create_info;
    pipeline_layout_create_info
      .setSetLayouts(descriptor_set_layout_);

    pipeline_layout_ = device.createPipelineLayout(pipeline_layout_create_info);

    // Color pipeline
    vk::VertexInputBindingDescription binding_description;
    vk::VertexInputAttributeDescription attribute_description;

    std::vector<vk::VertexInputBindingDescription> binding_descriptions;
    std::vector<vk::VertexInputAttributeDescription> attribute_descriptions;

    binding_description
      .setBinding(0)
      .setStride(3 * sizeof(float))
      .setInputRate(vk::VertexInputRate::eVertex);
    binding_descriptions.push_back(binding_description);

    attribute_description
      .setBinding(0)
      .setLocation(0)
      .setFormat(vk::Format::eR32G32B32Sfloat)
      .setOffset(0);
    attribute_descriptions.push_back(attribute_description);

    binding_description
      .setBinding(1)
      .setStride(3 * sizeof(float))
      .setInputRate(vk::VertexInputRate::eVertex);
    binding_descriptions.push_back(binding_description);

    attribute_description
      .setBinding(1)
      .setLocation(1)
      .setFormat(vk::Format::eR32G32B32Sfloat)
      .setOffset(0);
    attribute_descriptions.push_back(attribute_description);

    vk::PipelineVertexInputStateCreateInfo vertex_input_info;
    vertex_input_info
      .setVertexBindingDescriptions(binding_descriptions)
      .setVertexAttributeDescriptions(attribute_descriptions);

    const std::string base_dirpath = "C:\\workspace\\twopi\\src\\twopi\\shader";
    std::vector<vk::PipelineShaderStageCreateInfo> shader_stages;
    vk::ShaderModule vert_shader_module = CreateShaderModule(base_dirpath, "light_color.vert.spv");
    vk::ShaderModule frag_shader_module = CreateShaderModule(base_dirpath, "light_color.frag.spv");

    vk::PipelineShaderStageCreateInfo shader_stage;
    shader_stage
      .setStage(vk::ShaderStageFlagBits::eVertex)
      .setPName("main")
      .setModule(vert_shader_module);
    shader_stages.push_back(shader_stage);

    shader_stage
      .setStage(vk::ShaderStageFlagBits::eFragment)
      .setPName("main")
      .setModule(frag_shader_module);
    shader_stages.push_back(shader_stage);

    vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info;
    graphics_pipeline_create_info
      .setLayout(pipeline_layout_)
      .setRenderPass(render_pass_)
      .setSubpass(0)
      .setPVertexInputState(&vertex_input_info)
      .setPInputAssemblyState(&input_assembly_info)
      .setPViewportState(&viewport_state_info)
      .setPRasterizationState(&rasterization_info)
      .setPMultisampleState(&multisample_info)
      .setPDepthStencilState(nullptr)
      .setPColorBlendState(&color_blend_info)
      .setPDepthStencilState(&depth_stencil_info)
      .setPDynamicState(&dynamic_state_info)
      .setBasePipelineHandle(nullptr)
      .setBasePipelineIndex(-1)
      .setStages(shader_stages);

    color_pipeline_ = device.createGraphicsPipeline(nullptr, graphics_pipeline_create_info).value;

    device.destroyShaderModule(vert_shader_module);
    device.destroyShaderModule(frag_shader_module);
    shader_stages.clear();

    // Floor pipeline
    binding_description
      .setBinding(2)
      .setStride(2 * sizeof(float))
      .setInputRate(vk::VertexInputRate::eVertex);
    binding_descriptions.push_back(binding_description);

    attribute_description
      .setBinding(2)
      .setLocation(2)
      .setFormat(vk::Format::eR32G32Sfloat)
      .setOffset(0);
    attribute_descriptions.push_back(attribute_description);

    vertex_input_info
      .setVertexBindingDescriptions(binding_descriptions)
      .setVertexAttributeDescriptions(attribute_descriptions);

    vert_shader_module = CreateShaderModule(base_dirpath, "light_floor.vert.spv");
    frag_shader_module = CreateShaderModule(base_dirpath, "light_floor.frag.spv");

    shader_stage
      .setStage(vk::ShaderStageFlagBits::eVertex)
      .setPName("main")
      .setModule(vert_shader_module);
    shader_stages.push_back(shader_stage);

    shader_stage
      .setStage(vk::ShaderStageFlagBits::eFragment)
      .setModule(frag_shader_module);
    shader_stages.push_back(shader_stage);

    graphics_pipeline_create_info
      .setStages(shader_stages);

    floor_pipeline_ = device.createGraphicsPipeline(nullptr, graphics_pipeline_create_info).value;

    device.destroyShaderModule(vert_shader_module);
    device.destroyShaderModule(frag_shader_module);
    shader_stages.clear();

    // Tessellation shader
    binding_descriptions.clear();
    attribute_descriptions.clear();

    binding_description
      .setBinding(0)
      .setStride(3 * sizeof(float))
      .setInputRate(vk::VertexInputRate::eVertex);
    binding_descriptions.push_back(binding_description);

    attribute_description
      .setBinding(0)
      .setLocation(0)
      .setFormat(vk::Format::eR32G32B32Sfloat)
      .setOffset(0);
    attribute_descriptions.push_back(attribute_description);

    binding_description
      .setBinding(1)
      .setStride(3 * sizeof(float))
      .setInputRate(vk::VertexInputRate::eVertex);
    binding_descriptions.push_back(binding_description);

    attribute_description
      .setBinding(1)
      .setLocation(1)
      .setFormat(vk::Format::eR32G32B32Sfloat)
      .setOffset(0);
    attribute_descriptions.push_back(attribute_description);

    binding_description
      .setBinding(2)
      .setStride(3 * sizeof(float))
      .setInputRate(vk::VertexInputRate::eVertex);
    binding_descriptions.push_back(binding_description);

    attribute_description
      .setBinding(2)
      .setLocation(2)
      .setFormat(vk::Format::eR32G32B32Sfloat)
      .setOffset(0);
    attribute_descriptions.push_back(attribute_description);

    vertex_input_info
      .setVertexBindingDescriptions(binding_descriptions)
      .setVertexAttributeDescriptions(attribute_descriptions);

    vk::PipelineTessellationStateCreateInfo tessellation_stage_create_info;
    tessellation_stage_create_info
      .setPatchControlPoints(4);

    vert_shader_module = CreateShaderModule(base_dirpath, "surface.vert.spv");
    vk::ShaderModule tesc_shader_module = CreateShaderModule(base_dirpath, "surface.tesc.spv");
    vk::ShaderModule tese_shader_module = CreateShaderModule(base_dirpath, "surface.tese.spv");
    frag_shader_module = CreateShaderModule(base_dirpath, "surface.frag.spv");

    shader_stage
      .setStage(vk::ShaderStageFlagBits::eVertex)
      .setModule(vert_shader_module);
    shader_stages.push_back(shader_stage);
    
    shader_stage
      .setStage(vk::ShaderStageFlagBits::eTessellationControl)
      .setModule(tesc_shader_module);
    shader_stages.push_back(shader_stage);

    shader_stage
      .setStage(vk::ShaderStageFlagBits::eTessellationEvaluation)
      .setModule(tese_shader_module);
    shader_stages.push_back(shader_stage);

    shader_stage
      .setStage(vk::ShaderStageFlagBits::eFragment)
      .setModule(frag_shader_module);
    shader_stages.push_back(shader_stage);

    graphics_pipeline_create_info
      .setStages(shader_stages)
      .setPTessellationState(&tessellation_stage_create_info);

    rasterization_info
      .setCullMode(vk::CullModeFlagBits::eNone);

    input_assembly_info
      .setTopology(vk::PrimitiveTopology::ePatchList);

    // Polygon shader
    surface_tessellation_pipeline_ = device.createGraphicsPipeline(nullptr, graphics_pipeline_create_info).value;

    // Wireframe shader
    rasterization_info
      .setPolygonMode(vk::PolygonMode::eLine);

    surface_tessellation_wireframe_pipeline_ = device.createGraphicsPipeline(nullptr, graphics_pipeline_create_info).value;

    // Normal shader
    vk::ShaderModule geom_shader_module = CreateShaderModule(base_dirpath, "surface.geom.spv");

    shader_stage
      .setStage(vk::ShaderStageFlagBits::eGeometry)
      .setModule(geom_shader_module);
    shader_stages.push_back(shader_stage);

    rasterization_info
      .setPolygonMode(vk::PolygonMode::eFill);

    graphics_pipeline_create_info
      .setStages(shader_stages);

    device.destroyShaderModule(frag_shader_module);
    frag_shader_module = CreateShaderModule(base_dirpath, "surface_normal.frag.spv");
    shader_stage
      .setStage(vk::ShaderStageFlagBits::eFragment)
      .setModule(frag_shader_module);
    shader_stages[3] = shader_stage;

    graphics_pipeline_create_info
      .setStages(shader_stages);

    surface_tessellation_normal_pipeline_ = device.createGraphicsPipeline(nullptr, graphics_pipeline_create_info).value;

    device.destroyShaderModule(vert_shader_module);
    device.destroyShaderModule(frag_shader_module);
    device.destroyShaderModule(tesc_shader_module);
    device.destroyShaderModule(tese_shader_module);
    device.destroyShaderModule(geom_shader_module);
    shader_stages.clear();

    // Cubeskin support pipeline
    binding_descriptions.clear();
    attribute_descriptions.clear();

    binding_description
      .setBinding(0)
      .setStride(6 * sizeof(float))
      .setInputRate(vk::VertexInputRate::eVertex);
    binding_descriptions.push_back(binding_description);

    attribute_description
      .setBinding(0)
      .setLocation(0)
      .setFormat(vk::Format::eR32G32B32Sfloat)
      .setOffset(0);
    attribute_descriptions.push_back(attribute_description);

    vertex_input_info
      .setVertexBindingDescriptions(binding_descriptions)
      .setVertexAttributeDescriptions(attribute_descriptions);

    vert_shader_module = CreateShaderModule(base_dirpath, "cubeskin_support_lines.vert.spv");
    frag_shader_module = CreateShaderModule(base_dirpath, "cubeskin_support_lines.frag.spv");

    shader_stage
      .setStage(vk::ShaderStageFlagBits::eVertex)
      .setModule(vert_shader_module);
    shader_stages.push_back(shader_stage);

    shader_stage
      .setStage(vk::ShaderStageFlagBits::eFragment)
      .setModule(frag_shader_module);
    shader_stages.push_back(shader_stage);

    graphics_pipeline_create_info
      .setPTessellationState(nullptr)
      .setStages(shader_stages);

    input_assembly_info
      .setTopology(vk::PrimitiveTopology::eLineStrip)
      .setPrimitiveRestartEnable(true);

    cubeskin_support_lines_pipeline_ = device.createGraphicsPipeline(nullptr, graphics_pipeline_create_info).value;

    device.destroyShaderModule(vert_shader_module);
    device.destroyShaderModule(frag_shader_module);
    shader_stages.clear();
  }

  void DestroyGraphicsPipelines()
  {
    const auto device = context_->Device();

    device.destroyPipelineLayout(pipeline_layout_);
    device.destroyPipeline(color_pipeline_);
    device.destroyPipeline(floor_pipeline_);
    device.destroyPipeline(surface_tessellation_pipeline_);
    device.destroyPipeline(surface_tessellation_wireframe_pipeline_);
    device.destroyPipeline(surface_tessellation_normal_pipeline_);
    device.destroyPipeline(cubeskin_support_lines_pipeline_);
  }

  void CreateSynchronizationObjects()
  {
    const auto device = context_->Device();

    vk::SemaphoreCreateInfo semaphore_create_info;
    vk::FenceCreateInfo fence_create_info;

    fence_create_info
      .setFlags(vk::FenceCreateFlagBits::eSignaled);

    for (uint32_t i = 0; i < swapchain_->ImageCount(); i++)
    {
      image_available_semaphores_.emplace_back(device.createSemaphore(semaphore_create_info));
      render_finished_semaphores_.emplace_back(device.createSemaphore(semaphore_create_info));
      in_flight_fences_.emplace_back(device.createFence(fence_create_info));
    }

    images_in_flight_.resize(swapchain_->ImageCount());
  }

  void DestroySynchronizationObjects()
  {
    const auto device = context_->Device();

    for (auto& semaphore : image_available_semaphores_)
      device.destroySemaphore(semaphore);
    image_available_semaphores_.clear();

    for (auto& semaphore : render_finished_semaphores_)
      device.destroySemaphore(semaphore);
    render_finished_semaphores_.clear();

    for (auto& fence : in_flight_fences_)
      device.destroyFence(fence);
    in_flight_fences_.clear();
  }

  void PrepareDescriptors()
  {
    const auto device = context_->Device();
    const auto image_count = swapchain_->ImageCount();

    uniform_buffer_ = std::make_unique<vkl::UniformBuffer>(context_);

    for (uint32_t i = 0; i < image_count; i++)
      camera_ubos_.emplace_back(uniform_buffer_->Allocate<CameraUbo>());

    for (uint32_t i = 0; i < image_count; i++)
      light_ubos_.emplace_back(uniform_buffer_->Allocate<LightUbo>());

    // Dynamic uniform buffers
    for (uint32_t i = 0; i < image_count; i++)
      model_ubos_.emplace_back(uniform_buffer_->Allocate<ModelUbo>(num_objects_));

    for (uint32_t i = 0; i < image_count; i++)
      material_ubos_.emplace_back(uniform_buffer_->Allocate<MaterialUbo>(num_objects_));

    // Allocate descriptor sets
    std::vector<vk::DescriptorSetLayout> layouts(image_count, descriptor_set_layout_);
    vk::DescriptorSetAllocateInfo descriptor_set_allocate_info;
    descriptor_set_allocate_info
      .setSetLayouts(layouts)
      .setDescriptorPool(descriptor_pool_);

    descriptor_sets_ = device.allocateDescriptorSets(descriptor_set_allocate_info);

    std::vector<vk::DescriptorBufferInfo> buffer_infos(4);
    std::vector<vk::DescriptorImageInfo> image_infos(1);
    image_infos[0]
      .setSampler(sampler_)
      .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    std::vector<vk::WriteDescriptorSet> writes;
    vk::WriteDescriptorSet write;
    write
      .setDstBinding(0)
      .setDescriptorType(vk::DescriptorType::eUniformBuffer)
      .setDescriptorCount(1)
      .setDstArrayElement(0);
    writes.push_back(write);

    write
      .setDstBinding(1)
      .setDescriptorType(vk::DescriptorType::eUniformBufferDynamic);
    writes.push_back(write);

    write
      .setDstBinding(2)
      .setDescriptorType(vk::DescriptorType::eUniformBuffer);
    writes.push_back(write);

    write
      .setDstBinding(3)
      .setDescriptorType(vk::DescriptorType::eUniformBufferDynamic);
    writes.push_back(write);

    for (uint32_t i = 0; i < image_count; i++)
    {
      buffer_infos[0]
        .setBuffer(uniform_buffer_->Buffer())
        .setOffset(camera_ubos_[i].Offset())
        .setRange(camera_ubos_[i].Stride());

      buffer_infos[1]
        .setBuffer(uniform_buffer_->Buffer())
        .setOffset(model_ubos_[i].Offset())
        .setRange(model_ubos_[i].Stride());

      buffer_infos[2]
        .setBuffer(uniform_buffer_->Buffer())
        .setOffset(light_ubos_[i].Offset())
        .setRange(light_ubos_[i].Stride());

      buffer_infos[3]
        .setBuffer(uniform_buffer_->Buffer())
        .setOffset(material_ubos_[i].Offset())
        .setRange(material_ubos_[i].Stride());

      writes[0].setBufferInfo(buffer_infos[0]);
      writes[1].setBufferInfo(buffer_infos[1]);
      writes[2].setBufferInfo(buffer_infos[2]);
      writes[3].setBufferInfo(buffer_infos[3]);

      for (auto& write : writes)
        write.setDstSet(descriptor_sets_[i]);

      device.updateDescriptorSets(writes, nullptr);
    }
  }

  void PrepareResources()
  {
    const auto device = context_->Device();
    const auto physical_device = context_->PhysicalDevice();
    const auto queue = context_->Queue();

    // Primitives in CPU
    constexpr float floor_range = 30.f;
    floor_ = std::make_unique<Floor>(floor_range);
    constexpr int sphere_grid_size = 32;
    sphere_ = std::make_unique<Sphere>(sphere_grid_size);
    surface_ = std::make_unique<Surface>();

    // Vertex buffers
    floor_vbo_ = std::make_unique<VertexBuffer>(context_, floor_->NumVertices(), floor_->NumIndices());
    (*floor_vbo_)
      .AddAttribute<float, 3>(0)
      .AddAttribute<float, 3>(1)
      .AddAttribute<float, 2>(2)
      .Prepare();
    const uint64_t floor_buffer_size = floor_vbo_->BufferSize();

    sphere_vbo_ = std::make_unique<VertexBuffer>(context_, sphere_->NumVertices(), sphere_->NumIndices());
    (*sphere_vbo_)
      .AddAttribute<float, 3>(0)
      .AddAttribute<float, 3>(1)
      .Prepare();
    const auto sphere_buffer_size = sphere_vbo_->BufferSize();

    surface_vbo_ = std::make_unique<VertexBuffer>(context_, surface_->NumVertices(), surface_->NumIndices());
    (*surface_vbo_)
      .AddAttribute<float, 3>(0)
      .AddAttribute<float, 3>(1)
      .AddAttribute<float, 3>(2)
      .Prepare();
    const auto surface_buffer_size = surface_vbo_->BufferSize();

    context_->ToGpu(floor_->PositionBuffer(), floor_vbo_->Buffer(), floor_vbo_->Offset(0));
    context_->ToGpu(floor_->NormalBuffer(), floor_vbo_->Buffer(), floor_vbo_->Offset(1));
    context_->ToGpu(floor_->TexCoordBuffer(), floor_vbo_->Buffer(), floor_vbo_->Offset(2));
    context_->ToGpu(floor_->IndexBuffer(), floor_vbo_->Buffer(), floor_vbo_->IndexOffset());

    context_->ToGpu(sphere_->PositionBuffer(), sphere_vbo_->Buffer(), sphere_vbo_->Offset(0));
    context_->ToGpu(sphere_->NormalBuffer(), sphere_vbo_->Buffer(), sphere_vbo_->Offset(1));
    context_->ToGpu(sphere_->IndexBuffer(), sphere_vbo_->Buffer(), sphere_vbo_->IndexOffset());

    context_->ToGpu(surface_->PositionBuffer(), surface_vbo_->Buffer(), surface_vbo_->Offset(0));
    context_->ToGpu(surface_->VxBuffer(), surface_vbo_->Buffer(), surface_vbo_->Offset(1));
    context_->ToGpu(surface_->VyBuffer(), surface_vbo_->Buffer(), surface_vbo_->Offset(2));
    context_->ToGpu(surface_->IndexBuffer(), surface_vbo_->Buffer(), surface_vbo_->IndexOffset());

    // Cubeskin
    constexpr int segments = 32;
    constexpr int depth = 16;
    cubeskin_ = std::make_unique<Cubeskin>(context_, segments, depth);
  }

  void CleanupResources()
  {
    const auto device = context_->Device();
    const auto physical_device = context_->PhysicalDevice();

    floor_vbo_.reset();
    sphere_vbo_.reset();
    surface_vbo_.reset();
    uniform_buffer_.reset();

    cubeskin_.reset();
  }

  vk::ShaderModule CreateShaderModule(const std::string& dirpath, const std::string& filename)
  {
    const auto device = context_->Device();

    const auto filepath = dirpath + '/' + filename;
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);
    if (!file.is_open())
      throw core::Error("Failed to open file: " + filepath);

    size_t file_size = (size_t)file.tellg();
    std::vector<char> buffer(file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();

    std::vector<uint32_t> code;
    auto* int_ptr = reinterpret_cast<uint32_t*>(buffer.data());
    for (int i = 0; i < file_size / 4; i++)
      code.push_back(int_ptr[i]);

    vk::ShaderModuleCreateInfo shader_module_create_info;
    shader_module_create_info
      .setCode(code);

    return device.createShaderModule(shader_module_create_info);
  }

  void ChangeImageLayout(vk::CommandBuffer& command_buffer, vk::Image image, vk::ImageLayout old_layout, vk::ImageLayout new_layout, vk::ImageSubresourceRange subresource_range)
  {
    vk::ImageMemoryBarrier image_memory_barrier;
    image_memory_barrier
      .setOldLayout(old_layout)
      .setNewLayout(new_layout)
      .setImage(image)
      .setSubresourceRange(subresource_range);

    switch (old_layout)
    {
    case vk::ImageLayout::eUndefined:
      // Image layout is undefined (or does not matter)
      // Only valid as initial layout
      // No flags required, listed only for completeness
      image_memory_barrier.setSrcAccessMask(vk::AccessFlags{});
      break;
    }

    switch (new_layout)
    {
    case vk::ImageLayout::eShaderReadOnlyOptimal:
      // Image will be read in a shader (sampler, input attachment)
      // Make sure any writes to the image have been finished
      if (image_memory_barrier.srcAccessMask == vk::AccessFlags{})
        image_memory_barrier.setSrcAccessMask(vk::AccessFlagBits::eHostWrite | vk::AccessFlagBits::eTransferWrite);
      image_memory_barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);
      break;
    }

    command_buffer.pipelineBarrier(
      vk::PipelineStageFlagBits::eAllCommands,
      vk::PipelineStageFlagBits::eAllCommands,
      vk::DependencyFlags{},
      nullptr,
      nullptr,
      image_memory_barrier);
  }

  void AllocateDrawCommandBuffers()
  {
    const auto image_count = swapchain_->ImageCount();

    draw_command_buffers_ = context_->AllocateCommandBuffers(image_count);
  }

  void FreeDrawCommandBuffers()
  {
    context_->FreeCommandBuffers(std::move(draw_command_buffers_));
  }

  // Context
  std::shared_ptr<vkl::Context> context_;

  // Window
  GLFWwindow* glfw_window_handle_;
  uint32_t width_ = 0;
  uint32_t height_ = 0;
  uint32_t max_width_ = 0;
  uint32_t max_height_ = 0;

  // Swapchain
  std::unique_ptr<Swapchain> swapchain_;
  std::unique_ptr<vkl::Rendertarget> rendertarget_;

  // Swapchain render pass
  vk::RenderPass render_pass_;

  // Swapchain framebuffers
  std::vector<vk::Framebuffer> framebuffers_;

  // Sampler
  uint32_t mip_levels_ = 1;
  vk::Sampler sampler_;

  // Descriptor set
  vk::DescriptorSetLayout descriptor_set_layout_;
  vk::DescriptorPool descriptor_pool_;
  std::vector<vk::DescriptorSet> descriptor_sets_;

  // Pipelines
  vk::PipelineLayout pipeline_layout_;
  vk::Pipeline color_pipeline_;
  vk::Pipeline floor_pipeline_;
  vk::Pipeline cubeskin_support_lines_pipeline_;
  vk::Pipeline surface_tessellation_pipeline_;
  vk::Pipeline surface_tessellation_wireframe_pipeline_;
  vk::Pipeline surface_tessellation_normal_pipeline_;

  // Draw mode
  DrawMode draw_mode_ = DrawMode::SOLID;
  bool draw_normal_ = false;
  bool visible_ = true;

  // Uniform buffers per swapchain framebuffer
  uint32_t num_objects_ = 0;
  std::unique_ptr<UniformBuffer> uniform_buffer_;
  std::vector<UniformBuffer::Uniform<CameraUbo>> camera_ubos_;
  std::vector<UniformBuffer::Uniform<LightUbo>> light_ubos_;
  std::vector<UniformBuffer::Uniform<ModelUbo>> model_ubos_;
  std::vector<UniformBuffer::Uniform<MaterialUbo>> material_ubos_;

  CameraUbo camera_;
  LightUbo lights_;
  MaterialUbo material_;
  ModelUbo floor_model_;
  ModelUbo surface_model_;
  ModelUbo cubeskin_model_;
  ModelUbo light_model_;

  // Primitives
  std::unique_ptr<Floor> floor_;
  std::unique_ptr<Sphere> sphere_;
  std::unique_ptr<Surface> surface_;

  // Vertex buffers
  std::unique_ptr<VertexBuffer> floor_vbo_;
  std::unique_ptr<VertexBuffer> sphere_vbo_;
  std::unique_ptr<VertexBuffer> surface_vbo_;

  // Model
  std::unique_ptr<Cubeskin> cubeskin_;

  // Draw command buffers
  std::vector<vk::CommandBuffer> draw_command_buffers_;

  // Synchronization
  static constexpr uint32_t max_frames_in_flight_ = 2;
  uint32_t current_frame_ = 0;
  std::vector<vk::Semaphore> image_available_semaphores_;
  std::vector<vk::Semaphore> render_finished_semaphores_;
  std::vector<vk::Fence> in_flight_fences_;
  std::vector<vk::Fence> images_in_flight_;
};

Engine::Engine(std::shared_ptr<window::Window> window)
  : impl_(std::make_unique<Impl>(window))
{
}

Engine::~Engine() = default;

void Engine::Draw(core::Duration duration)
{
  impl_->Draw(duration);
}

void Engine::Resize(int width, int height)
{
  impl_->Resize(width, height);
}

void Engine::UpdateLights(const std::vector<std::shared_ptr<scene::Light>>& lights)
{
  impl_->UpdateLights(lights);
}

void Engine::UpdateCamera(std::shared_ptr<scene::Camera> camera)
{
  impl_->UpdateCamera(camera);
}

void Engine::SetDrawWireframe()
{
  impl_->SetDrawWireframe();
}

void Engine::SetDrawNormal(bool draw_normal)
{
  impl_->SetDrawNormal(draw_normal);
}

void Engine::SetDrawSolid()
{
  impl_->SetDrawSolid();
}
}
}
