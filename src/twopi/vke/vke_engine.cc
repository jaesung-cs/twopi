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

  struct Texture
  {
    std::unique_ptr<vke::Image> image;
    vkw::ImageView image_view;
  };

  struct Mesh
  {
    std::unique_ptr<vke::Buffer> vertex_buffer;
    std::unique_ptr<vke::Buffer> index_buffer;
    std::unique_ptr<vke::Buffer> instance_buffer;
    uint64_t normal_offset = 0;
    uint64_t tex_coord_offset = 0;
    uint64_t instance_offset = 0;
    uint64_t num_indices = 0;
    uint64_t num_instances = 0;
  };

public:
  Impl() = delete;

  Impl(std::shared_ptr<window::Window> window)
  {
    width_ = window->Width();
    height_ = window->Height();

    PrintInstanceInfo();

    // Create instance
    instance_ = vkw::Instance::Creator{}
      .AddGlfwRequiredExtensions()
#ifdef __APPLE__
      .AddGetPhysicalDeviceProperties2Extension()
#endif
      .EnableValidationLayer()
      .Create();

    // Create surface
    const auto glfw_window = std::dynamic_pointer_cast<window::GlfwWindow>(window)->Handle();
    surface_ = vkw::Surface::Creator{ instance_, glfw_window }.Create();

    // TODO: pick the most suitable device, now simply use physical device of index 0
    const auto physical_devices = instance_.PhysicalDevices();
    physical_device_ = physical_devices[0];
    PrintPhysicalDeviceInfo();

    // Create device
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

    // Transient command buffer pool
    transient_command_pool_ = vkw::CommandPool::Creator{ device_ }
      .SetTransient()
      .SetQueue(graphics_queue_)
      .Create();

    // Create memory manager
    memory_manager_ = std::make_shared<vke::MemoryManager>(device_);

    // Create shaders
    CreateShaders();

    // Create swapchain
    CreateSwapchain();

    descriptor_set_layout_ = vkw::DescriptorSetLayout::Creator{ device_ }
      .AddUniformBuffer()
      .AddSampler()
      .Create();

    pipeline_cache_ = vkw::PipelineCache::Creator{ device_ }.Create();

    CreateGraphicsPipeline();

    // Allocate staging buffer
    constexpr uint64_t staging_buffer_size = 256 * 1024 * 1024; // 256MB
    auto staging_buffer = vkw::Buffer::Creator{ device_ }
      .SetSize(256 * 1024 * 1024)
      .SetTransferSrcBuffer()
      .Create();

    staging_buffer_ = std::make_unique<vke::Buffer>(
      std::move(staging_buffer),
      memory_manager_->AllocateHostVisibleMemory(staging_buffer_size));

    // Load mesh and texture
    meshes_.emplace_back(LoadMesh("C:\\workspace\\twopi\\resources\\among_us_obj\\among us_scaled.obj"));
    meshes_.emplace_back(LoadMesh("C:\\workspace\\twopi\\resources\\viking_room\\viking_room.obj"));
    textures_.emplace_back(LoadTexture("C:\\workspace\\twopi\\resources\\among_us_obj\\Plastic_4K_Diffuse.jpg"));
    textures_.emplace_back(LoadTexture("C:\\workspace\\twopi\\resources\\viking_room\\viking_room.png"));

    // Create image sampler
    sampler_ = vkw::Sampler::Creator{ device_ }
      .SetMipLevels(3)
      .EnableAnisotropy(physical_device_)
      .Create();

    // Create uniform
    CreateUniformBuffers();
    CreateDescriptorSets();

    // Commands
    command_pool_ = vkw::CommandPool::Creator{ device_ }
      .SetQueue(graphics_queue_)
      .Create();

    CreateCommandBuffers();

    // Synchronizations
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
    CleanupUniformBuffers();
    CleanupDescriptors();
    CleanupPipeline();
    CleanupCommandBuffers();

    transient_command_pool_.Destroy();

    vert_shader_.Destroy();
    frag_shader_.Destroy();

    memory_manager_.reset();

    pipeline_cache_.Destroy();

    for (auto& texture : textures_)
    {
      texture.image.reset();
      texture.image_view.Destroy();
    }
    textures_.clear();

    sampler_.Destroy();

    descriptor_set_layout_.Destroy();

    for (auto& mesh : meshes_)
    {
      mesh.vertex_buffer.reset();
      mesh.index_buffer.reset();
      mesh.instance_buffer.reset();
    }
    meshes_.clear();

    staging_buffer_.reset();

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

    auto* ptr = uniform_buffer_map_ + (256 * image_index);
    std::memcpy(ptr, glm::value_ptr(projection_matrix_), mat4_size);
    std::memcpy(ptr + mat4_size, glm::value_ptr(view_matrix_), mat4_size);
    std::memcpy(ptr + mat4_size * 2, glm::value_ptr(model_matrix), mat4_size);

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
  void PrintInstanceInfo()
  {
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
  }

  void PrintPhysicalDeviceInfo()
  {
    std::cout << "Physical devices:" << std::endl;
    std::cout << "  " << physical_device_.Properties().deviceName << std::endl;
    if (physical_device_.Properties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
      std::cout << "    " << "Discrete GPU" << std::endl;
    if (physical_device_.Features().geometryShader)
      std::cout << "    " << "Has Geometry Shader" << std::endl;
    if (physical_device_.Features().samplerAnisotropy)
      std::cout << "    " << "Has Sampler Anisotropy" << std::endl;

    std::cout << "    Extensions:" << std::endl;
    const auto extensions = physical_device_.Extensions();
    for (const auto& extension : extensions)
      std::cout << "      " << extension.extensionName << std::endl;

    std::cout << "    Memory properties:" << std::endl;
    const auto memory_properties = physical_device_.MemoryProperties();

    for (uint32_t i = 0; i < memory_properties.memoryHeapCount; i++)
    {
      std::cout
        << "      Heap " << i << ": " << memory_properties.memoryHeaps[i].size << " bytes ("
        << memory_properties.memoryHeaps[i].size / 1024 / 1024 << " MB)" << std::endl
        << "        Memories: " << std::endl;

      for (uint32_t j = 0; j < memory_properties.memoryTypeCount; j++)
      {
        if (memory_properties.memoryTypes[j].heapIndex == i)
        {
          std::cout << "          Memory Type " << j << ": "
            << vk::to_string(memory_properties.memoryTypes[j].propertyFlags) << std::endl;
        }
      }
    }
  }

  Mesh LoadMesh(const std::string& mesh_filepath)
  {
    Mesh vk_mesh;

    // Load mesh
    geometry::MeshLoader mesh_loader{};
    auto mesh = mesh_loader.Load(mesh_filepath);

    const auto& mesh_vertex_buffer = mesh->Vertices();
    const auto& mesh_normal_buffer = mesh->Normals();
    const auto& mesh_tex_coords_buffer = mesh->TexCoords();
    const auto mesh_buffer_size = (mesh_vertex_buffer.size() + mesh_normal_buffer.size() + mesh_tex_coords_buffer.size()) * sizeof(float);

    const auto& mesh_index_buffer = mesh->Indices();
    const auto mesh_index_buffer_size = mesh_index_buffer.size() * sizeof(uint32_t);

    // Instance buffer
    constexpr int grid_size = 5;
    std::vector<glm::mat4> models;
    for (int x = 0; x < grid_size; x++)
    {
      for (int y = 0; y < grid_size; y++)
      {
        for (int z = 0; z < grid_size; z++)
        {
          glm::mat4 m = glm::mat4(1.f);
          m[3][0] = x * 2.f;
          m[3][1] = y * 2.f;
          m[3][2] = z * 2.f;
          models.emplace_back(std::move(m));
        }
      }
    }
    const float* mesh_instance_buffer = glm::value_ptr(models[0]);
    const auto mesh_instance_buffer_size = models.size() * 16 * sizeof(float);

    auto ptr = static_cast<unsigned char*>(staging_buffer_->Map());

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

    staging_buffer_->Unmap();

    vk_mesh.normal_offset = mesh_vertex_buffer.size() * sizeof(float);
    vk_mesh.tex_coord_offset = vk_mesh.normal_offset + mesh_normal_buffer.size() * sizeof(float);
    vk_mesh.instance_offset = vk_mesh.tex_coord_offset + mesh_tex_coords_buffer.size() * sizeof(float);
    vk_mesh.num_indices = mesh_index_buffer.size();
    vk_mesh.num_instances = models.size();

    auto vertex_buffer = vkw::Buffer::Creator{ device_ }
      .SetSize(mesh_buffer_size)
      .SetTransferDstBuffer()
      .SetVertexBuffer()
      .Create();

    vk_mesh.vertex_buffer = std::make_unique<vke::Buffer>(
      std::move(vertex_buffer),
      memory_manager_->AllocateDeviceLocalMemory(mesh_buffer_size));

    auto index_buffer = vkw::Buffer::Creator{ device_ }
      .SetSize(mesh_index_buffer_size)
      .SetTransferDstBuffer()
      .SetIndexBuffer()
      .Create();

    vk_mesh.index_buffer = std::make_unique<vke::Buffer>(
      std::move(index_buffer),
      memory_manager_->AllocateDeviceLocalMemory(mesh_index_buffer_size));

    auto instance_buffer = vkw::Buffer::Creator{ device_ }
      .SetSize(mesh_instance_buffer_size)
      .SetTransferDstBuffer()
      .SetVertexBuffer()
      .Create();

    vk_mesh.instance_buffer = std::make_unique<vke::Buffer>(
      std::move(instance_buffer),
      memory_manager_->AllocateDeviceLocalMemory(mesh_instance_buffer_size));

    // Transfer from staging buffer to device local memory
    auto copy_command = vkw::CommandBuffer::Allocator{ device_, transient_command_pool_ }.Allocate(1)[0];
    copy_command
      .BeginOneTime()
      .CopyBuffer(*staging_buffer_, *vk_mesh.vertex_buffer, mesh_buffer_size)
      .CopyBuffer(*staging_buffer_, mesh_buffer_size, *vk_mesh.index_buffer, 0, mesh_index_buffer_size)
      .CopyBuffer(*staging_buffer_, mesh_buffer_size + mesh_index_buffer_size, *vk_mesh.instance_buffer, 0, mesh_instance_buffer_size)
      .End();
    graphics_queue_.Submit(copy_command);
    graphics_queue_.WaitIdle();

    copy_command.Free();

    return vk_mesh;
  }

  Texture LoadTexture(const std::string& image_filepath)
  {
    Texture texture;

    // Load image
    geometry::ImageLoader image_loader{};
    auto texture_image = image_loader.Load<uint8_t>(image_filepath);

    // Create vulkan image
    auto image = vkw::Image::Creator{ device_ }
      .SetSize(texture_image->Width(), texture_image->Height())
      .SetMipLevels(3)
      .SetTransferSrc()
      .Create();

    texture.image = std::make_unique<vke::Image>(
      std::move(image),
      memory_manager_->AllocateDeviceLocalMemory(image.RequiredMemorySize()));

    texture.image_view = vkw::ImageView::Creator{ device_ }
      .SetImage(*texture.image)
      .SetMipLevels(3)
      .Create();

    // Copy image pixels to image staging buffer
    auto* image_ptr = static_cast<uint8_t*>(staging_buffer_->Map());
    std::memcpy(image_ptr, texture_image->Buffer().data(), texture_image->Buffer().size());
    staging_buffer_->Unmap();

    // One time image layout transition
    auto layout_transition_commands = vkw::CommandBuffer::Allocator{ device_, transient_command_pool_ }.Allocate(2);
    ChangeImageLayout(layout_transition_commands[0], *texture.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 3);

    // Transfer from staging buffer to device local memory
    auto copy_command = vkw::CommandBuffer::Allocator{ device_, transient_command_pool_ }.Allocate(1)[0];
    copy_command
      .BeginOneTime()
      .CopyBuffer(*staging_buffer_, *texture.image)
      .End();
    graphics_queue_.Submit(copy_command);
    graphics_queue_.WaitIdle();

    GenerateMipmaps(layout_transition_commands[1], *texture.image, 3);

    for (auto& layout_transition_command : layout_transition_commands)
      layout_transition_command.Free();

    copy_command.Free();

    return texture;
  }

  void CreateShaders()
  {
#ifdef _WIN32
    const std::string dirpath = "C:\\workspace\\twopi\\src";
#elif __APPLE__
    const std::string dirpath = "/Users/jaesung/workspace/twopi/src";
#endif

    vkw::ShaderModule::Creator shader_module_creator{ device_ };
    vert_shader_ = shader_module_creator.Load(dirpath + "/twopi/shader/mesh_instance.vert.spv").Create();
    frag_shader_ = shader_module_creator.Load(dirpath + "/twopi/shader/mesh_instance.frag.spv").Create();
  }

  void RecreateSwapchain()
  {
    device_.WaitIdle();

    CleanupSwapchain();
    CleanupUniformBuffers();
    CleanupDescriptors();
    CleanupPipeline();
    CleanupCommandBuffers();

    CreateSwapchain();
    CreateUniformBuffers();
    CreateDescriptorSets();
    CreateGraphicsPipeline();
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

    // Create image views from swapchain images
    CreateSwapchainImageViews();

    // Create depth buffer
    CreateDepthBuffer();

    // Color rendertarget
    CreateRendertargetImage();

    // Create render pass
    CreateRenderPass();

    // Create framebuffers
    CreateFramebuffers();
  }

  void CreateSwapchainImageViews()
  {
    for (const auto& swapchain_image : swapchain_images_)
    {
      auto swapchain_image_view = vkw::ImageView::Creator{ device_ }.SetImage(swapchain_image).Create();
      swapchain_image_views_.emplace_back(std::move(swapchain_image_view));
    }
  }

  void CreateDepthBuffer()
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

  void CreateRenderPass()
  {
    render_pass_ = vkw::RenderPass::Creator{ device_ }
      .SetFormat(swapchain_images_[0])
      .SetMultisample4()
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

  void CreateUniformBuffers()
  {
    // Uniform buffers
    const int uniform_buffer_size = 256 * swapchain_image_views_.size();
    auto uniform_buffer = vkw::Buffer::Creator{ device_ }
      .SetSize(uniform_buffer_size)
      .SetUniformBuffer()
      .Create();

    uniform_buffer_ = std::make_unique<vke::Buffer>(
      std::move(uniform_buffer),
      memory_manager_->AllocatePersistenlyMappedMemory(device_.MemoryRequirements(uniform_buffer).size));

    uniform_buffer_map_ = static_cast<unsigned char*>(uniform_buffer_->Map());
  }

  void CreateDescriptorSets()
  {
    descriptor_pool_ = vkw::DescriptorPool::Creator{ device_ }
      .AddUniformBuffer()
      .AddSampler()
      .SetSize(textures_.size() * swapchain_image_views_.size())
      .Create();

    for (int i = 0; i < textures_.size(); i++)
    {
      auto descriptor_framebuffer_sets = vkw::DescriptorSet::Allocator{ device_, descriptor_pool_ }
        .SetLayout(descriptor_set_layout_)
        .SetSize(swapchain_image_views_.size())
        .Allocate();

      for (int j = 0; j < descriptor_framebuffer_sets.size(); j++)
        descriptor_framebuffer_sets[j].Update(*uniform_buffer_, j * 256, sizeof(float) * 16 * 3, textures_[i].image_view, sampler_);

      descriptor_sets_.emplace_back(std::move(descriptor_framebuffer_sets));
    }
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
  
  void CreateCommandBuffers()
  {
    command_buffers_ = vkw::CommandBuffer::Allocator{ device_, command_pool_ }
      .Allocate(swapchain_framebuffers_.size());

    for (int i = 0; i < command_buffers_.size(); i++)
    {
      auto& command_buffer = command_buffers_[i];
      command_buffer
        .Begin()
        .BeginRenderPass(render_pass_, swapchain_framebuffers_[i]);

      for (int j = 0; j < meshes_.size(); j++)
      {
        command_buffer
          .BindVertexBuffers({ *meshes_[j].vertex_buffer, *meshes_[j].vertex_buffer, *meshes_[j].vertex_buffer }, { 0, meshes_[j].normal_offset, meshes_[j].tex_coord_offset })
          .BindVertexBuffers({ *meshes_[j].instance_buffer }, { 0 }, 3)
          .BindIndexBuffer(*meshes_[j].index_buffer)
          .BindPipeline(pipeline_)
          .BindDescriptorSets(pipeline_layout_, { descriptor_sets_[j][i] })
          .DrawIndexed(meshes_[j].num_indices, meshes_[j].num_instances);
      }

      command_buffer
        .EndRenderPass()
        .End();
    }
  }

  void CleanupSwapchain()
  {
    for (auto& swapchain_framebuffer : swapchain_framebuffers_)
      swapchain_framebuffer.Destroy();
    swapchain_framebuffers_.clear();

    render_pass_.Destroy();

    rendertarget_image_.reset();
    rendertarget_image_view_.Destroy();

    depth_image_.reset();
    depth_image_view_.Destroy();

    for (auto& swapchain_image_view : swapchain_image_views_)
      swapchain_image_view.Destroy();
    swapchain_image_views_.clear();

    swapchain_.Destroy();
  }

  void CleanupUniformBuffers()
  {
    uniform_buffer_->Unmap();
    uniform_buffer_.reset();
  }

  void CleanupDescriptors()
  {
    descriptor_pool_.Destroy();
    descriptor_sets_.clear();
  }

  void CleanupCommandBuffers()
  {
    for (auto& command_buffer : command_buffers_)
      command_buffer.Free();
    command_buffers_.clear();
  }

  void CleanupPipeline()
  {
    pipeline_.Destroy();
    pipeline_layout_.Destroy();
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

  // Memory
  std::shared_ptr<MemoryManager> memory_manager_;
  std::unique_ptr<vke::Buffer> staging_buffer_;

  // Swapchain resources
  vkw::Surface surface_;
  vkw::Swapchain swapchain_;
  std::vector<vkw::Image> swapchain_images_;
  std::vector<vkw::ImageView> swapchain_image_views_;
  std::unique_ptr<vke::Image> depth_image_;
  vkw::ImageView depth_image_view_;
  std::unique_ptr<vke::Image> rendertarget_image_;
  vkw::ImageView rendertarget_image_view_;
  vkw::RenderPass render_pass_;
  std::vector<vkw::Framebuffer> swapchain_framebuffers_;
  std::vector<vkw::CommandBuffer> command_buffers_;

  // Rendering & presentation synchronization
  size_t current_frame_ = 0;
  std::vector<vkw::Semaphore> image_available_semaphores_;
  std::vector<vkw::Semaphore> render_finished_semaphores_;
  std::vector<vkw::Fence> in_flight_fences_;
  std::vector<vkw::Fence> images_in_flight_;

  // Uniform buffer
  std::unique_ptr<vke::Buffer> uniform_buffer_;
  unsigned char* uniform_buffer_map_;

  glm::mat4 projection_matrix_;
  glm::mat4 view_matrix_;

  // Descriptors
  vkw::DescriptorPool descriptor_pool_;
  std::vector<std::vector<vkw::DescriptorSet>> descriptor_sets_; // [texture_id][swapchain_image_index]

  // Graphics pipeline
  vkw::ShaderModule vert_shader_;
  vkw::ShaderModule frag_shader_;
  vkw::DescriptorSetLayout descriptor_set_layout_;
  vkw::DescriptorSetLayout sampler_layout_;
  vkw::PipelineCache pipeline_cache_;
  vkw::PipelineLayout pipeline_layout_;
  vkw::GraphicsPipeline pipeline_;

  // Commands
  vkw::CommandPool transient_command_pool_;
  vkw::CommandPool command_pool_;

  // Vertex attributes
  std::vector<Mesh> meshes_;

  // Texture
  std::vector<Texture> textures_;
  vkw::Sampler sampler_;

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
