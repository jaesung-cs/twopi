#include <twopi/vk/vk_engine.h>

#include <iostream>

#include <twopi/core/error.h>
#include <twopi/window/window.h>
#include <twopi/window/glfw_window.h>
#include <twopi/geometry/image_loader.h>
#include <twopi/geometry/image.h>
#include <twopi/scene/camera.h>
#include <twopi/vk/vk_instance.h>
#include <twopi/vk/vk_physical_device.h>
#include <twopi/vk/vk_device.h>
#include <twopi/vk/vk_queue.h>
#include <twopi/vk/vk_surface.h>
#include <twopi/vk/vk_swapchain.h>
#include <twopi/vk/vk_image.h>
#include <twopi/vk/vk_image_view.h>
#include <twopi/vk/vk_shader_module.h>
#include <twopi/vk/vk_pipeline_layout.h>
#include <twopi/vk/vk_render_pass.h>
#include <twopi/vk/vk_graphics_pipeline.h>
#include <twopi/vk/vk_framebuffer.h>
#include <twopi/vk/vk_command_pool.h>
#include <twopi/vk/vk_command_buffer.h>
#include <twopi/vk/vk_semaphore.h>
#include <twopi/vk/vk_fence.h>
#include <twopi/vk/vk_buffer.h>
#include <twopi/vk/vk_device_memory.h>
#include <twopi/vk/vk_descriptor_set_layout.h>
#include <twopi/vk/vk_descriptor_pool.h>
#include <twopi/vk/vk_descriptor_set.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace twopi
{
namespace vkw
{
class Engine::Impl
{
private:
  static constexpr int max_frames_in_flight_ = 2;

public:
  Impl() = delete;

  Impl(std::shared_ptr<window::Window> window)
  {
    width_ = window->Width();
    height_ = window->Height();

    const auto extensions = Instance::Extensions();
    std::cout << "Available instance extensions:" << std::endl;
    for (const auto& extension : extensions)
      std::cout << "  " << extension.extensionName << std::endl;
    std::cout << std::endl;

    const auto layers = Instance::Layers();
    std::cout << "Available instance layers:" << std::endl;
    for (const auto& layer : layers)
      std::cout << "  " << layer.layerName << ": " << layer.description << std::endl;
    std::cout << std::endl;

    instance_ = Instance::Creator{}
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

      std::cout << "    Extensions:" << std::endl;
      const auto extensions = physical_device.Extensions();
      for (const auto& extension : extensions)
        std::cout << "      " << extension.extensionName << std::endl;
    }

    const auto glfw_window = std::dynamic_pointer_cast<window::GlfwWindow>(window)->Handle();
    surface_ = Surface::Creator{ instance_, glfw_window }.Create();

    // TODO: pick the most suitable device, now simply use physical device of index 0
    physical_device_ = physical_devices[0];

    device_ = Device::Creator{ physical_device_ }
      .AddGraphicsQueue()
      .AddPresentQueue(surface_)
      .AddSwapchainExtension()
#ifdef __APPLE__
      .AddPortabilitySubsetExtension()
#endif
      .Create();

    graphics_queue_ = device_.Queue(0);
    present_queue_ = device_.Queue(1);

    CreateSwapchain();

    CreateSwapchainImageViews();

#ifdef _WIN32
    const std::string dirpath = "C:\\workspace\\twopi\\src";
#elif __APPLE__
    const std::string dirpath = "/Users/jaesung/workspace/twopi/src";
#endif
    ShaderModule::Creator shader_module_creator{ device_ };
    vert_shader_ = shader_module_creator.Load(dirpath + "/twopi/shader/vk/triangle_3d.vert.spv").Create();
    frag_shader_ = shader_module_creator.Load(dirpath + "/twopi/shader/vk/triangle_3d.frag.spv").Create();

    CreateRenderPass();

    uniform_buffer_layout_ = DescriptorSetLayout::Creator{ device_ }.Create();

    CreateGraphicsPipeline();

    CreateFramebuffers();

    constexpr auto buffer_size = sizeof(float) * 24;
    constexpr auto index_buffer_size = sizeof(uint32_t) * 6;
    vertex_staging_buffer_ = Buffer::Creator{ device_ }
      .SetSize(buffer_size + index_buffer_size)
      .SetTransferSrcBuffer()
      .Create();

    vertex_staging_buffer_memory_ = DeviceMemory::Allocator{ device_ }
      .SetHostVisibleCoherentMemory(vertex_staging_buffer_, physical_device_)
      .Allocate();

    vertex_staging_buffer_.Bind(vertex_staging_buffer_memory_);

    auto ptr = static_cast<unsigned char*>(vertex_staging_buffer_memory_.Map());

    float vertex_buffer[] = {
      // Position
      -0.5f, -0.5f, 0.f,
      0.5f, -0.5f, 0.f,
      -0.5f, 0.5f, 0.f,
      0.5f, 0.5f, 0.f,
      // Color
      1.f, 0.f, 0.f,
      0.f, 1.f, 0.f,
      0.f, 0.f, 1.f,
      1.f, 1.f, 1.f,
    };
    std::memcpy(ptr, vertex_buffer, buffer_size);

    uint32_t index_buffer[] = {
      0, 1, 2, 2, 3, 1
    };
    std::memcpy(ptr + buffer_size, index_buffer, index_buffer_size);

    vertex_staging_buffer_memory_.Unmap();

    vertex_buffer_ = Buffer::Creator{ device_ }
      .SetSize(buffer_size)
      .SetTransferDstBuffer()
      .SetVertexBuffer()
      .Create();

    vertex_buffer_memory_ = DeviceMemory::Allocator{ device_ }
      .SetDeviceLocalMemory(vertex_buffer_, physical_device_)
      .Allocate();

    vertex_buffer_.Bind(vertex_buffer_memory_);

    index_buffer_ = Buffer::Creator{ device_ }
      .SetSize(buffer_size)
      .SetTransferDstBuffer()
      .SetIndexBuffer()
      .Create();

    index_buffer_memory_ = DeviceMemory::Allocator{ device_ }
      .SetDeviceLocalMemory(index_buffer_, physical_device_)
      .Allocate();

    index_buffer_.Bind(index_buffer_memory_);

    // Load image
    const std::string image_filepath = "C:\\workspace\\twopi\\resources\\viking_room.png";
    geometry::ImageLoader image_loader{};
    auto image = image_loader.Load<uint8_t>(image_filepath);

    // Create vulkan image
    image_ = vkw::Image::Creator{ device_ }
      .SetSize(image->Width(), image->Height())
      .Create();

    image_memory_ = vkw::DeviceMemory::Allocator{ device_ }
      .SetDeviceLocalMemory(image_, physical_device_)
      .Allocate();

    image_.Bind(image_memory_);

    image_staging_buffer_ = vkw::Buffer::Creator{ device_ }
      .SetTransferSrcBuffer()
      .SetSize(image->Width() * image->Height() * 4)
      .Create();

    image_staging_buffer_memory_ = vkw::DeviceMemory::Allocator{ device_ }
      .SetHostVisibleCoherentMemory(image_staging_buffer_, physical_device_)
      .Allocate();

    image_staging_buffer_.Bind(image_staging_buffer_memory_);

    // One time transfer for vertex buffer
    auto transient_command_pool = CommandPool::Creator{ device_ }
      .SetTransient()
      .Create();

    auto copy_commands = CommandBuffer::Allocator{ device_, transient_command_pool }.Allocate(2);
    copy_commands[0]
      .BeginOneTime()
      .CopyBuffer(vertex_staging_buffer_, vertex_buffer_, buffer_size)
      .CopyBuffer(vertex_staging_buffer_, buffer_size, index_buffer_, 0, index_buffer_size)
      .End();

    graphics_queue_.Submit(copy_commands[0]);

    // One time transfer for image
    auto single_commands = CommandBuffer::Allocator{ device_, transient_command_pool }.Allocate(2);
    ChangeImageLayout(single_commands[0], image_, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

    copy_commands[1]
      .BeginOneTime()
      .CopyBuffer(image_staging_buffer_, image_)
      .End();
    graphics_queue_.Submit(copy_commands[1]);
    graphics_queue_.WaitIdle();

    ChangeImageLayout(single_commands[1], image_, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    for (auto& single_command : single_commands)
      single_command.Free();
    single_commands.clear();

    for (auto& copy_command : copy_commands)
      copy_command.Free();
    copy_commands.clear();

    transient_command_pool.Destroy();
    
    CreateUniformBuffers();

    command_pool_ = CommandPool::Creator{ device_ }
      .SetQueue(graphics_queue_)
      .Create();

    CreateCommandBuffers();

    auto semaphore_creator = Semaphore::Creator{ device_ };
    auto fence_creator = Fence::Creator{ device_ };
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

    image_.Destroy();
    image_memory_.Free();
    image_staging_buffer_.Destroy();
    image_staging_buffer_memory_.Free();

    uniform_buffer_layout_.Destroy();

    vertex_staging_buffer_.Destroy();
    vertex_staging_buffer_memory_.Free();
    vertex_buffer_.Destroy();
    vertex_buffer_memory_.Free();
    index_buffer_.Destroy();
    index_buffer_memory_.Free();

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

    auto* ptr = static_cast<unsigned char*>(uniform_buffer_memories_[image_index].Map());
    std::memcpy(ptr, glm::value_ptr(projection_matrix_), mat4_size);
    std::memcpy(ptr + mat4_size, glm::value_ptr(view_matrix_), mat4_size);
    std::memcpy(ptr + mat4_size * 2, glm::value_ptr(model_matrix), mat4_size);
    uniform_buffer_memories_[image_index].Unmap();

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
    CreateUniformBuffers();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandBuffers();
  }

  void CreateSwapchain()
  {
    swapchain_ = Swapchain::Creator{ physical_device_, device_, surface_ }
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
      auto swapchain_image_view = ImageView::Creator{ device_ }.SetImage(swapchain_image).Create();
      swapchain_image_views_.emplace_back(std::move(swapchain_image_view));
    }
  }

  void CreateUniformBuffers()
  {
    // Uniform buffers
    constexpr int uniform_buffer_size = sizeof(float) * 16 * 3;
    auto uniform_buffer_creator = Buffer::Creator{ device_ }
      .SetSize(uniform_buffer_size)
      .SetUniformBuffer();

    for (int i = 0; i < swapchain_image_views_.size(); i++)
    {
      auto uniform_buffer = uniform_buffer_creator.Create();
      auto uniform_buffer_memory = DeviceMemory::Allocator{ device_ }
        .SetHostVisibleCoherentMemory(uniform_buffer, physical_device_)
        .Allocate();
      uniform_buffer.Bind(uniform_buffer_memory);

      uniform_buffers_.emplace_back(std::move(uniform_buffer));
      uniform_buffer_memories_.emplace_back(std::move(uniform_buffer_memory));
    }

    uniform_descriptor_pool_ = DescriptorPool::Creator{ device_ }
      .SetSize(swapchain_image_views_.size())
      .Create();

    uniform_descriptor_sets_ = DescriptorSet::Allocator{ device_, uniform_descriptor_pool_ }
      .SetLayout(uniform_buffer_layout_)
      .SetSize(swapchain_image_views_.size())
      .Allocate();

    for (int i = 0; i < uniform_descriptor_sets_.size(); i++)
      uniform_descriptor_sets_[i].Update(uniform_buffers_[i]);
  }

  void CreateRenderPass()
  {
    render_pass_ = RenderPass::Creator{ device_ }
      .SetFormat(swapchain_images_[0])
      .Create();
  }

  void CreateGraphicsPipeline()
  {
    pipeline_layout_ = PipelineLayout::Creator{ device_ }
      .SetLayouts({ uniform_buffer_layout_ })
      .Create();

    pipeline_ = GraphicsPipeline::Creator{ device_ }
      .SetShader(vert_shader_, frag_shader_)
      .SetVertexInput({ {0, 3}, {1, 3} })
      .SetViewport(width_, height_)
      .SetPipelineLayout(pipeline_layout_)
      .SetRenderPass(render_pass_)
      .Create();
  }

  void CreateFramebuffers()
  {
    auto framebuffer_creator = Framebuffer::Creator{ device_ };
    for (const auto& swapchain_image_view : swapchain_image_views_)
    {
      auto swapchain_framebuffer = framebuffer_creator
        .SetAttachment(swapchain_image_view)
        .SetExtent(width_, height_)
        .SetRenderPass(render_pass_)
        .Create();

      swapchain_framebuffers_.emplace_back(std::move(swapchain_framebuffer));
    }
  }

  void CreateCommandBuffers()
  {
    command_buffers_ = CommandBuffer::Allocator{ device_, command_pool_ }
      .Allocate(swapchain_framebuffers_.size());

    for (int i = 0; i < command_buffers_.size(); i++)
    {
      auto& command_buffer = command_buffers_[i];
      command_buffer
        .Begin()
        .BeginRenderPass(render_pass_, swapchain_framebuffers_[i])
        .BindVertexBuffers({ vertex_buffer_, vertex_buffer_ }, { 0, 9 * sizeof(float) })
        .BindIndexBuffer(index_buffer_)
        .BindPipeline(pipeline_)
        .BindDescriptorSets(pipeline_layout_, { uniform_descriptor_sets_[i] })
        .DrawIndexed(6, 1)
        .EndRenderPass()
        .End();
    }
  }

  void CleanupSwapchain()
  {
    for (auto& uniform_buffer : uniform_buffers_)
      uniform_buffer.Destroy();
    uniform_buffers_.clear();

    for (auto& uniform_buffer_memory : uniform_buffer_memories_)
      uniform_buffer_memory.Free();
    uniform_buffer_memories_.clear();

    uniform_descriptor_pool_.Destroy();

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

  void ChangeImageLayout(vkw::CommandBuffer command_buffer, vkw::Image image, vk::ImageLayout old_layout, vk::ImageLayout new_layout)
  {
    command_buffer
      .BeginOneTime()
      .PipelineBarrier(image, old_layout, new_layout)
      .End();

    graphics_queue_.Submit(command_buffer);
    graphics_queue_.WaitIdle();
  }

private:
  vkw::Instance instance_;
  vkw::PhysicalDevice physical_device_;
  vkw::Device device_;

  vkw::Queue graphics_queue_;
  vkw::Queue present_queue_;

  vkw::Surface surface_;

  vkw::Swapchain swapchain_;
  std::vector<vkw::Image> swapchain_images_;
  std::vector<vkw::ImageView> swapchain_image_views_;
  std::vector<vkw::Framebuffer> swapchain_framebuffers_;

  vkw::ShaderModule vert_shader_;
  vkw::ShaderModule frag_shader_;
  vkw::DescriptorSetLayout uniform_buffer_layout_;
  vkw::PipelineLayout pipeline_layout_;
  vkw::RenderPass render_pass_;
  vkw::GraphicsPipeline pipeline_;

  vkw::CommandPool command_pool_;
  std::vector<vkw::CommandBuffer> command_buffers_;

  size_t current_frame_ = 0;
  std::vector<vkw::Semaphore> image_available_semaphores_;
  std::vector<vkw::Semaphore> render_finished_semaphores_;
  std::vector<vkw::Fence> in_flight_fences_;
  std::vector<vkw::Fence> images_in_flight_;

  vkw::Buffer vertex_staging_buffer_;
  vkw::DeviceMemory vertex_staging_buffer_memory_;
  vkw::Buffer vertex_buffer_;
  vkw::DeviceMemory vertex_buffer_memory_;
  vkw::Buffer index_buffer_;
  vkw::DeviceMemory index_buffer_memory_;
  std::vector<vkw::Buffer> uniform_buffers_;
  std::vector<vkw::DeviceMemory> uniform_buffer_memories_;
  vkw::DescriptorPool uniform_descriptor_pool_;
  std::vector<vkw::DescriptorSet> uniform_descriptor_sets_;

  glm::mat4 projection_matrix_;
  glm::mat4 view_matrix_;

  vkw::Image image_;
  vkw::DeviceMemory image_memory_;
  vkw::Buffer image_staging_buffer_;
  vkw::DeviceMemory image_staging_buffer_memory_;

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
