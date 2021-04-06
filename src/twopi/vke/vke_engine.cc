#include <twopi/vke/vke_engine.h>

#include <iostream>

#include <twopi/core/error.h>
#include <twopi/window/window.h>
#include <twopi/window/glfw_window.h>
#include <twopi/geometry/image_loader.h>
#include <twopi/geometry/image.h>
#include <twopi/geometry/mesh_loader.h>
#include <twopi/geometry/mesh.h>
#include <twopi/scene/camera.h>
#include <twopi/vkw/vkw_instance.h>
#include <twopi/vkw/vkw_physical_device.h>
#include <twopi/vkw/vkw_device.h>
#include <twopi/vkw/vkw_queue.h>
#include <twopi/vkw/vkw_surface.h>
#include <twopi/vkw/vkw_swapchain.h>
#include <twopi/vkw/vkw_image.h>
#include <twopi/vkw/vkw_image_view.h>
#include <twopi/vkw/vkw_shader_module.h>
#include <twopi/vkw/vkw_pipeline_cache.h>
#include <twopi/vkw/vkw_pipeline_layout.h>
#include <twopi/vkw/vkw_render_pass.h>
#include <twopi/vkw/vkw_graphics_pipeline.h>
#include <twopi/vkw/vkw_framebuffer.h>
#include <twopi/vkw/vkw_command_pool.h>
#include <twopi/vkw/vkw_command_buffer.h>
#include <twopi/vkw/vkw_semaphore.h>
#include <twopi/vkw/vkw_fence.h>
#include <twopi/vkw/vkw_buffer.h>
#include <twopi/vkw/vkw_device_memory.h>
#include <twopi/vkw/vkw_descriptor_set_layout.h>
#include <twopi/vkw/vkw_descriptor_pool.h>
#include <twopi/vkw/vkw_descriptor_set.h>
#include <twopi/vkw/vkw_sampler.h>
#include <twopi/vke/vke_buffer.h>
#include <twopi/vke/vke_image.h>
#include <twopi/vke/vke_memory.h>
#include <twopi/vke/vke_memory_manager.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace twopi
{
namespace vke
{
class Engine::Impl
{
private:
  static constexpr int max_frames_in_flight_ = 2;
  static constexpr int alignment_ = 256;

public:
  Impl() = delete;

  Impl(std::shared_ptr<window::Window> window)
  {
    width_ = window->Width();
    height_ = window->Height();

    const auto extensions = vkw::Instance::Extensions();
    std::cout << "Available instance extensions:" << std::endl;
    for (const auto& extension : extensions)
      std::cout << "  " << extension.extensionName << std::endl;
    std::cout << std::endl;

    const auto layers = vkw::Instance::Layers();
    std::cout << "Available instance layers:" << std::endl;
    for (const auto& layer : layers)
      std::cout << "  " << layer.layerName << ": " << layer.description << std::endl;
    std::cout << std::endl;

    instance_ = vkw::Instance::Creator{}
      .AddGlfwRequiredExtensions()
#ifdef __APPLE__
      .AddGetPhysicalDeviceProperties2Extension()
#endif
      .EnableValidationLayer()
      .Create();

    std::cout << "Physical devices:" << std::endl;
    const auto physical_devices = instance_.PhysicalDevices();
    for (const auto& physical_device : physical_devices)
    {
      std::cout << "  " << physical_device.Properties().deviceName << std::endl;
      if (physical_device.Properties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
        std::cout << "    " << "Discrete GPU" << std::endl;
      if (physical_device.Features().geometryShader)
        std::cout << "    " << "Has Geometry Shader" << std::endl;
      if (physical_device.Features().samplerAnisotropy)
        std::cout << "    " << "Has Sampler Anisotropy" << std::endl;

      std::cout << "    Extensions:" << std::endl;
      const auto extensions = physical_device.Extensions();
      for (const auto& extension : extensions)
        std::cout << "      " << extension.extensionName << std::endl;

      std::cout << "    Memory properties:" << std::endl;
      const auto memory_properties = physical_device.MemoryProperties();

      for (int i = 0; i < memory_properties.memoryHeapCount; i++)
      {
        std::cout
          << "      Heap " << i << ": " << memory_properties.memoryHeaps[i].size << " bytes ("
          << memory_properties.memoryHeaps[i].size / 1024 / 1024 << " MB)" << std::endl
          << "        Memories: " << std::endl;

        for (int j = 0; j < memory_properties.memoryTypeCount; j++)
        {
          if (memory_properties.memoryTypes[j].heapIndex == i)
          {
            std::cout << "          Memory Type " << j << ": "
              << vk::to_string(memory_properties.memoryTypes[j].propertyFlags) << std::endl;
          }
        }
      }
    }

    const auto glfw_window = std::dynamic_pointer_cast<window::GlfwWindow>(window)->Handle();
    surface_ = vkw::Surface::Creator{ instance_, glfw_window }.Create();

    // TODO: pick the most suitable device, now simply use physical device of index 0
    physical_device_ = physical_devices[0];

    device_ = vkw::Device::Creator{ physical_device_ }
      .AddGraphicsQueue()
      .AddPresentQueue(surface_)
      .AddSwapchainExtension()
#ifdef __APPLE__
      .AddPortabilitySubsetExtension()
#endif
      .Create();

    graphics_queue_ = device_.Queue(0);
    present_queue_ = device_.Queue(1);

    memory_manager_ = std::make_shared<vke::MemoryManager>(device_);

    CreateSwapchain();

    CreateSwapchainImageViews();

#ifdef _WIN32
    const std::string dirpath = "C:\\workspace\\twopi\\src";
#elif __APPLE__
    const std::string dirpath = "/Users/jaesung/workspace/twopi/src";
#endif
    vkw::ShaderModule::Creator shader_module_creator{ device_ };
    vert_shader_ = shader_module_creator.Load(dirpath + "/twopi/shader/mesh_instance.vert.spv").Create();
    frag_shader_ = shader_module_creator.Load(dirpath + "/twopi/shader/mesh_instance.frag.spv").Create();

    CreateRenderPass();

    descriptor_set_layout_ = vkw::DescriptorSetLayout::Creator{ device_ }
      .AddUniformBuffer()
      .AddSampler()
      .Create();

    pipeline_cache_ = vkw::PipelineCache::Creator{ device_ }.Create();

    CreateGraphicsPipeline();

    // Load image
    const std::string mesh_filepath = "C:\\workspace\\twopi\\resources\\viking_room.obj";
    geometry::MeshLoader mesh_loader{};
    auto mesh = mesh_loader.Load(mesh_filepath);

    const auto& mesh_vertex_buffer = mesh->Vertices();
    const auto& mesh_normal_buffer = mesh->Normals();
    const auto& mesh_tex_coords_buffer = mesh->TexCoords();
    const auto mesh_buffer_size = (mesh_vertex_buffer.size() + mesh_normal_buffer.size() + mesh_tex_coords_buffer.size()) * sizeof(float);

    const auto& mesh_index_buffer = mesh->Indices();
    const auto mesh_index_buffer_size = mesh_index_buffer.size() * sizeof(uint32_t);

    constexpr int grid_size = 5;
    std::vector<glm::mat4> models;
    for (int x = 0; x < grid_size; x++)
    {
      for (int y = 0; y < grid_size; y++)
      {
        for (int z = 0; z < grid_size; z++)
        {
          glm::mat4 m = glm::mat4(1.f);
          m[3][0] = x;
          m[3][1] = y;
          m[3][2] = z;
          models.emplace_back(std::move(m));
        }
      }
    }

    const float* mesh_instance_buffer = glm::value_ptr(models[0]);
    const auto mesh_instance_buffer_size = models.size() * 16 * sizeof(float);

    auto vertex_staging_buffer = vkw::Buffer::Creator{ device_ }
      .SetSize(mesh_buffer_size + mesh_index_buffer_size + mesh_instance_buffer_size)
      .SetTransferSrcBuffer()
      .Create();

    vertex_staging_buffer_ = std::make_unique<vke::Buffer>(
      std::move(vertex_staging_buffer),
      memory_manager_->AllocateHostVisibleMemory(mesh_buffer_size + mesh_index_buffer_size + mesh_instance_buffer_size));

    auto ptr = static_cast<unsigned char*>(vertex_staging_buffer_->Map());

    std::memcpy(ptr, mesh_vertex_buffer.data(), mesh_vertex_buffer.size() * sizeof(float));
    ptr += mesh_vertex_buffer.size() * sizeof(float);

    std::memcpy(ptr, mesh_normal_buffer.data(), mesh_normal_buffer.size() * sizeof(float));
    ptr += mesh_normal_buffer.size() * sizeof(float);

    std::memcpy(ptr, mesh_tex_coords_buffer.data(), mesh_tex_coords_buffer.size() * sizeof(float));
    ptr += mesh_tex_coords_buffer.size() * sizeof(float);

    std::memcpy(ptr, mesh_index_buffer.data(), mesh_index_buffer.size() * sizeof(uint32_t));
    ptr += mesh_index_buffer.size() * sizeof(uint32_t);

    std::memcpy(ptr, mesh_instance_buffer, mesh_instance_buffer_size);
    ptr += mesh_instance_buffer_size;

    vertex_staging_buffer_->Unmap();

    normal_offset_ = mesh_vertex_buffer.size() * sizeof(float);
    tex_coord_offset_ = normal_offset_ + mesh_normal_buffer.size() * sizeof(float);
    instance_offset_ = tex_coord_offset_ + mesh_tex_coords_buffer.size() * sizeof(float);
    num_indices_ = mesh_index_buffer.size();
    num_instances_ = models.size();

    auto vertex_buffer = vkw::Buffer::Creator{ device_ }
      .SetSize(mesh_buffer_size)
      .SetTransferDstBuffer()
      .SetVertexBuffer()
      .Create();

    vertex_buffer_ = std::make_unique<vke::Buffer>(
      std::move(vertex_buffer),
      memory_manager_->AllocateDeviceLocalMemory(mesh_buffer_size));

    auto index_buffer = vkw::Buffer::Creator{ device_ }
      .SetSize(mesh_index_buffer_size)
      .SetTransferDstBuffer()
      .SetIndexBuffer()
      .Create();

    index_buffer_ = std::make_unique<vke::Buffer>(
      std::move(index_buffer),
      memory_manager_->AllocateDeviceLocalMemory(mesh_index_buffer_size));

    auto instance_buffer = vkw::Buffer::Creator{ device_ }
      .SetSize(mesh_instance_buffer_size)
      .SetTransferDstBuffer()
      .SetVertexBuffer()
      .Create();

    instance_buffer_ = std::make_unique<vke::Buffer>(
      std::move(instance_buffer),
      memory_manager_->AllocateDeviceLocalMemory(mesh_instance_buffer_size));

    // Load image
    const std::string image_filepath = "C:\\workspace\\twopi\\resources\\viking_room.png";
    geometry::ImageLoader image_loader{};
    auto texture_image = image_loader.Load<uint8_t>(image_filepath);

    // Create vulkan image
    auto image = vkw::Image::Creator{ device_ }
      .SetSize(texture_image->Width(), texture_image->Height())
      .SetMipLevels(3)
      .SetTransferSrc()
      .Create();

    image_ = std::make_unique<vke::Image>(
      std::move(image),
      memory_manager_->AllocateDeviceLocalMemory(image.RequiredMemorySize()));

    image_view_ = vkw::ImageView::Creator{ device_ }
      .SetImage(*image_)
      .SetMipLevels(3)
      .Create();

    auto image_staging_buffer = vkw::Buffer::Creator{ device_ }
      .SetTransferSrcBuffer()
      .SetSize(image.Width() * image.Height() * 4)
      .Create();

    image_staging_buffer_ = std::make_unique<vke::Buffer>(
      std::move(image_staging_buffer),
      memory_manager_->AllocateHostVisibleMemory(image.Width() * image.Height() * 4));

    // Copy image pixels to image staging buffer
    auto* image_ptr = static_cast<uint8_t*>(image_staging_buffer_->Map());
    std::memcpy(image_ptr, texture_image->Buffer().data(), texture_image->Buffer().size());
    image_staging_buffer_->Unmap();

    // One time transfer for vertex buffer
    auto transient_command_pool = vkw::CommandPool::Creator{ device_ }
      .SetTransient()
      .Create();

    auto copy_commands = vkw::CommandBuffer::Allocator{ device_, transient_command_pool }.Allocate(2);
    copy_commands[0]
      .BeginOneTime()
      .CopyBuffer(*vertex_staging_buffer_, *vertex_buffer_, mesh_buffer_size)
      .CopyBuffer(*vertex_staging_buffer_, mesh_buffer_size, *index_buffer_, 0, mesh_index_buffer_size)
      .CopyBuffer(*vertex_staging_buffer_, mesh_buffer_size + mesh_index_buffer_size, *instance_buffer_, 0, mesh_instance_buffer_size)
      .End();
    graphics_queue_.Submit(copy_commands[0]);

    // One time transfer for image
    auto single_commands = vkw::CommandBuffer::Allocator{ device_, transient_command_pool }.Allocate(2);
    ChangeImageLayout(single_commands[0], *image_, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 3);

    copy_commands[1]
      .BeginOneTime()
      .CopyBuffer(*image_staging_buffer_, *image_)
      .End();
    graphics_queue_.Submit(copy_commands[1]);
    graphics_queue_.WaitIdle();

    GenerateMipmaps(single_commands[1], *image_, 3);

    // Depth buffer
    auto depth_image = vkw::Image::Creator{ device_ }
      .SetDepthStencilImage()
      .SetMultisample4()
      .SetSize(width_, height_)
      .Create();

    depth_image_ = std::make_unique<vke::Image>(
      std::move(depth_image),
      memory_manager_->AllocateDeviceLocalMemory(depth_image.RequiredMemorySize()));

    depth_image_view_ = vkw::ImageView::Creator{ device_ }
      .SetDepthImage(*depth_image_)
      .Create();

    // Color rendertarget
    CreateRendertargetImage();

    for (auto& single_command : single_commands)
      single_command.Free();
    single_commands.clear();

    for (auto& copy_command : copy_commands)
      copy_command.Free();
    copy_commands.clear();

    transient_command_pool.Destroy();

    // Create image sampler
    sampler_ = vkw::Sampler::Creator{ device_ }
      .SetMipLevels(3)
      .EnableAnisotropy(physical_device_)
      .Create();

    CreateFramebuffers();

    CreateUniformBuffers();

    CreateDescriptorSets();

    command_pool_ = vkw::CommandPool::Creator{ device_ }
      .SetQueue(graphics_queue_)
      .Create();

    CreateCommandBuffers();

    auto semaphore_creator = vkw::Semaphore::Creator{ device_ };
    auto fence_creator = vkw::Fence::Creator{ device_ };
    for (int i = 0; i < max_frames_in_flight_; i++)
    {
      image_available_semaphores_.emplace_back(semaphore_creator.Create());
      render_finished_semaphores_.emplace_back(semaphore_creator.Create());
      in_flight_fences_.emplace_back(fence_creator.Create());
    }

    // Refernces to actual fences
    images_in_flight_.resize(swapchain_framebuffers_.size());
  }

  ~Impl()
  {
    device_.WaitIdle();

    CleanupSwapchain();

    memory_manager_.reset();

    pipeline_cache_.Destroy();

    image_.reset();
    image_view_.Destroy();
    image_staging_buffer_.reset();

    sampler_.Destroy();

    descriptor_set_layout_.Destroy();

    vertex_staging_buffer_.reset();
    vertex_buffer_.reset();
    index_buffer_.reset();
    instance_buffer_.reset();

    for (auto& in_flight_fence : in_flight_fences_)
      in_flight_fence.Destroy();
    in_flight_fences_.clear();

    for (auto& image_available_semaphore : image_available_semaphores_)
      image_available_semaphore.Destroy();
    image_available_semaphores_.clear();

    for (auto& render_finished_semaphore : render_finished_semaphores_)
      render_finished_semaphore.Destroy();
    render_finished_semaphores_.clear();

    command_pool_.Destroy();

    vert_shader_.Destroy();
    frag_shader_.Destroy();

    surface_.Destroy();
    device_.Destroy();
    instance_.Destroy();
  }

  void UpdateCamera(std::shared_ptr<scene::Camera> camera)
  {
    projection_matrix_ = camera->ProjectionMatrix();
    projection_matrix_[1][1] *= -1.f;

    view_matrix_ = camera->ViewMatrix();
  }

  void Draw()
  {
    in_flight_fences_[current_frame_].Wait();

    const auto [image_index, result] = device_.AcquireNextImage(swapchain_, image_available_semaphores_[current_frame_]);
    if (result == vk::Result::eErrorOutOfDateKHR)
    {
      RecreateSwapchain();
      return;
    }
    else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
      throw core::Error("Failed to acquire swapchain image.");

    if (images_in_flight_[image_index])
      images_in_flight_[image_index].Wait();
    images_in_flight_[image_index] = in_flight_fences_[current_frame_];

    in_flight_fences_[current_frame_].Reset();

    // Update uniform buffer
    constexpr uint64_t mat4_size = sizeof(float) * 16;
    glm::mat4 model_matrix{ 1.f };

    auto* ptr = static_cast<unsigned char*>(uniform_buffers_[image_index]->Map());
    std::memcpy(ptr, glm::value_ptr(projection_matrix_), mat4_size);
    std::memcpy(ptr + mat4_size, glm::value_ptr(view_matrix_), mat4_size);
    std::memcpy(ptr + mat4_size * 2, glm::value_ptr(model_matrix), mat4_size);
    uniform_buffers_[image_index]->Unmap();

    graphics_queue_.Submit(command_buffers_[image_index], { image_available_semaphores_[current_frame_] }, { render_finished_semaphores_[current_frame_] }, in_flight_fences_[current_frame_]);

    auto present_result = present_queue_.Present(swapchain_, image_index, { render_finished_semaphores_[current_frame_] });
    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
      RecreateSwapchain();
    else if (result != vk::Result::eSuccess)
      throw core::Error("Failed to present swapchain image.");

    current_frame_ = (current_frame_ + 1) % max_frames_in_flight_;
  }

  void Resize(int width, int height)
  {
    width_ = width;
    height_ = height;

    RecreateSwapchain();
  }

private:
  void RecreateSwapchain()
  {
    device_.WaitIdle();

    CleanupSwapchain();

    CreateSwapchain();
    CreateSwapchainImageViews();
    RecreateDepthBuffer();
    CreateRendertargetImage();
    CreateUniformBuffers();
    CreateDescriptorSets();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandBuffers();
  }

  void CreateSwapchain()
  {
    swapchain_ = vkw::Swapchain::Creator{ physical_device_, device_, surface_ }
      .SetTripleBuffering()
      .SetDefaultFormat()
      .SetExtent(width_, height_)
      .Create();

    swapchain_images_ = swapchain_.Images();
  }

  void CreateSwapchainImageViews()
  {
    for (const auto& swapchain_image : swapchain_images_)
    {
      auto swapchain_image_view = vkw::ImageView::Creator{ device_ }.SetImage(swapchain_image).Create();
      swapchain_image_views_.emplace_back(std::move(swapchain_image_view));
    }
  }

  void RecreateDepthBuffer()
  {
    auto depth_image = vkw::Image::Creator{ device_ }
      .SetDepthStencilImage()
      .SetMultisample4()
      .SetSize(width_, height_)
      .Create();

    depth_image_ = std::make_unique<vke::Image>(
      std::move(depth_image),
      memory_manager_->AllocateDeviceLocalMemory(depth_image.RequiredMemorySize()));

    depth_image_view_ = vkw::ImageView::Creator{ device_ }
      .SetDepthImage(*depth_image_)
      .Create();
  }

  void CreateRendertargetImage()
  {
    auto rendertarget_image = vkw::Image::Creator{ device_ }
      .SetSize(width_, height_)
      .SetMultisample4()
      .SetTransientColorAttachment()
      .SetFormat(swapchain_images_[0].Format())
      .Create();

    rendertarget_image_ = std::make_unique<vke::Image>(
      std::move(rendertarget_image),
      memory_manager_->AllocateDeviceLocalMemory(rendertarget_image.RequiredMemorySize()));

    rendertarget_image_view_ = vkw::ImageView::Creator{ device_ }
      .SetImage(*rendertarget_image_)
      .Create();
  }

  void CreateUniformBuffers()
  {
    // Uniform buffers
    constexpr int uniform_buffer_size = sizeof(float) * 16 * 3;
    auto uniform_buffer_creator = vkw::Buffer::Creator{ device_ }
      .SetSize(uniform_buffer_size)
      .SetUniformBuffer();

    for (int i = 0; i < swapchain_image_views_.size(); i++)
    {
      auto buffer = uniform_buffer_creator.Create();

      auto uniform_buffer = std::make_unique<vke::Buffer>(
        std::move(buffer),
        memory_manager_->AllocateHostVisibleMemory(uniform_buffer_size));

      uniform_buffers_.emplace_back(std::move(uniform_buffer));
    }
  }

  void CreateDescriptorSets()
  {
    descriptor_pool_ = vkw::DescriptorPool::Creator{ device_ }
      .AddUniformBuffer()
      .AddSampler()
      .SetSize(swapchain_image_views_.size())
      .Create();

    descriptor_sets_ = vkw::DescriptorSet::Allocator{ device_, descriptor_pool_ }
      .SetLayout(descriptor_set_layout_)
      .SetSize(swapchain_image_views_.size())
      .Allocate();

    for (int i = 0; i < descriptor_sets_.size(); i++)
      descriptor_sets_[i].Update(*uniform_buffers_[i], image_view_, sampler_);
  }

  void CreateRenderPass()
  {
    render_pass_ = vkw::RenderPass::Creator{ device_ }
      .SetFormat(swapchain_images_[0])
      .SetMultisample4()
      .Create();
  }

  void CreateGraphicsPipeline()
  {
    pipeline_layout_ = vkw::PipelineLayout::Creator{ device_ }
      .SetLayouts({ descriptor_set_layout_ })
      .Create();

    pipeline_ = vkw::GraphicsPipeline::Creator{ device_ }
      .SetPipelineCache(pipeline_cache_)
      .SetMultisample4()
      .SetShader(vert_shader_, frag_shader_)
      .SetVertexInput({ {0, 3}, {1, 3}, {2, 2} })
      .SetInstanceInput({ {3, 4, 4} })
      .SetViewport(width_, height_)
      .SetPipelineLayout(pipeline_layout_)
      .SetRenderPass(render_pass_)
      .Create();
  }
  
  void CreateFramebuffers()
  {
    auto framebuffer_creator = vkw::Framebuffer::Creator{ device_ };
    for (const auto& swapchain_image_view : swapchain_image_views_)
    {
      auto swapchain_framebuffer = framebuffer_creator
        .SetAttachments({ rendertarget_image_view_, depth_image_view_, swapchain_image_view })
        .SetExtent(width_, height_)
        .SetRenderPass(render_pass_)
        .Create();

      swapchain_framebuffers_.emplace_back(std::move(swapchain_framebuffer));
    }
  }

  void CreateCommandBuffers()
  {
    command_buffers_ = vkw::CommandBuffer::Allocator{ device_, command_pool_ }
      .Allocate(swapchain_framebuffers_.size());

    for (int i = 0; i < command_buffers_.size(); i++)
    {
      auto& command_buffer = command_buffers_[i];
      command_buffer
        .Begin()
        .BeginRenderPass(render_pass_, swapchain_framebuffers_[i])
        .BindVertexBuffers({ *vertex_buffer_, *vertex_buffer_, *vertex_buffer_ }, { 0, normal_offset_, tex_coord_offset_ })
        .BindVertexBuffers({ *instance_buffer_ }, { 0 }, 3)
        .BindIndexBuffer(*index_buffer_)
        .BindPipeline(pipeline_)
        .BindDescriptorSets(pipeline_layout_, { descriptor_sets_[i] })
        .DrawIndexed(num_indices_, num_instances_)
        .EndRenderPass()
        .End();
    }
  }

  void CleanupSwapchain()
  {
    depth_image_.reset();
    depth_image_view_.Destroy();

    rendertarget_image_.reset();
    rendertarget_image_view_.Destroy();

    for (auto& uniform_buffer : uniform_buffers_)
      uniform_buffer.reset();
    uniform_buffers_.clear();

    descriptor_pool_.Destroy();
    descriptor_sets_.clear();

    for (auto& swapchain_framebuffer : swapchain_framebuffers_)
      swapchain_framebuffer.Destroy();
    swapchain_framebuffers_.clear();

    for (auto& command_buffer : command_buffers_)
      command_buffer.Free();
    command_buffers_.clear();

    pipeline_.Destroy();
    pipeline_layout_.Destroy();
    render_pass_.Destroy();

    for (auto& swapchain_image_view : swapchain_image_views_)
      swapchain_image_view.Destroy();
    swapchain_image_views_.clear();

    swapchain_.Destroy();
  }

  void ChangeImageLayout(vkw::CommandBuffer command_buffer, vkw::Image image, vk::ImageLayout old_layout, vk::ImageLayout new_layout, int mip_levels = 1)
  {
    command_buffer
      .BeginOneTime()
      .PipelineBarrier(image, old_layout, new_layout, mip_levels)
      .End();

    graphics_queue_.Submit(command_buffer);
    graphics_queue_.WaitIdle();
  }

  void GenerateMipmaps(vkw::CommandBuffer command_buffer, vkw::Image image, int mip_levels)
  {
    command_buffer.BeginOneTime();

    auto width = image.Width();
    auto height = image.Height();
    for (int i = 1; i < mip_levels; i++)
    {
      command_buffer
        .PipelineBarrier(image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eTransferSrcOptimal, 1, i - 1)
        .BlitImage(image, width, height, i)
        .PipelineBarrier(image, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 1, i - 1);

      width /= 2;
      height /= 2;
    }

    command_buffer
      .PipelineBarrier(image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 1, mip_levels - 1)
      .End();

    graphics_queue_.Submit(command_buffer);
    graphics_queue_.WaitIdle();
  }

private:
  vkw::Instance instance_;

  // Device
  vkw::PhysicalDevice physical_device_;
  vkw::Device device_;
  vkw::Queue graphics_queue_;
  vkw::Queue present_queue_;

  vkw::Surface surface_;

  // Memory
  std::shared_ptr<MemoryManager> memory_manager_;

  // Depth buffer
  std::unique_ptr<vke::Image> depth_image_;
  vkw::ImageView depth_image_view_;

  // Multisample image
  std::unique_ptr<vke::Image> rendertarget_image_;
  vkw::ImageView rendertarget_image_view_;

  // Vertex attributes
  std::unique_ptr<vke::Buffer> vertex_staging_buffer_;
  std::unique_ptr<vke::Buffer> vertex_buffer_;
  std::unique_ptr<vke::Buffer> index_buffer_;
  std::unique_ptr<vke::Buffer> instance_buffer_;
  uint64_t normal_offset_ = 0;
  uint64_t tex_coord_offset_ = 0;
  uint64_t instance_offset_ = 0;
  uint64_t num_indices_ = 0;
  uint64_t num_instances_ = 0;

  // Texture
  std::unique_ptr<vke::Image> image_;
  vkw::ImageView image_view_;
  std::unique_ptr<vke::Buffer> image_staging_buffer_;
  vkw::Sampler sampler_;

  // Uniform buffer
  std::vector<std::unique_ptr<vke::Buffer>> uniform_buffers_;
  glm::mat4 projection_matrix_;
  glm::mat4 view_matrix_;

  // Descriptors
  vkw::DescriptorPool descriptor_pool_;
  std::vector<vkw::DescriptorSet> descriptor_sets_;

  // Swapchain
  vkw::Swapchain swapchain_;
  std::vector<vkw::Image> swapchain_images_;
  std::vector<vkw::ImageView> swapchain_image_views_;
  std::vector<vkw::Framebuffer> swapchain_framebuffers_;

  // Graphics pipeline
  vkw::ShaderModule vert_shader_;
  vkw::ShaderModule frag_shader_;
  vkw::DescriptorSetLayout descriptor_set_layout_;
  vkw::DescriptorSetLayout sampler_layout_;
  vkw::PipelineCache pipeline_cache_;
  vkw::PipelineLayout pipeline_layout_;
  vkw::RenderPass render_pass_;
  vkw::GraphicsPipeline pipeline_;

  // Commands
  vkw::CommandPool command_pool_;
  std::vector<vkw::CommandBuffer> command_buffers_;

  // Rendering & presentation synchronization
  size_t current_frame_ = 0;
  std::vector<vkw::Semaphore> image_available_semaphores_;
  std::vector<vkw::Semaphore> render_finished_semaphores_;
  std::vector<vkw::Fence> in_flight_fences_;
  std::vector<vkw::Fence> images_in_flight_;

  // Window
  int width_ = 0;
  int height_ = 0;
};

Engine::Engine(std::shared_ptr<window::Window> window)
{
  impl_ = std::make_unique<Impl>(window);
}

Engine::~Engine() = default;

void Engine::Draw()
{
  impl_->Draw();
}

void Engine::Resize(int width, int height)
{
  impl_->Resize(width, height);
}

void Engine::UpdateCamera(std::shared_ptr<scene::Camera> camera)
{
  impl_->UpdateCamera(camera);
}
}
}
