#include <twopi/vk/vk_engine.h>

#include <iostream>

#include <twopi/window/window.h>
#include <twopi/window/glfw_window.h>
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
    }

    const auto glfw_window = std::dynamic_pointer_cast<window::GlfwWindow>(window)->Handle();
    surface_ = Surface::Creator{ instance_, glfw_window }.Create();

    // TODO: pick the most suitable device, now simply use physical device of index 0
    physical_device_ = physical_devices[0];

    device_ = Device::Creator{ physical_device_ }
      .AddGraphicsQueue()
      .AddPresentQueue(surface_)
      .AddSwapchainExtension()
      .Create();

    graphics_queue_ = device_.Queue(0);
    present_queue_ = device_.Queue(1);

    swapchain_ = Swapchain::Creator{ physical_device_, device_, surface_ }
      .SetTripleBuffering()
      .SetDefaultFormat()
      .SetExtent(window->Width(), window->Height())
      .Create();

    swapchain_images_ = swapchain_.Images();

    for (const auto& swapchain_image : swapchain_images_)
    {
      const auto swapchain_image_view = ImageView::Creator{ device_ }.SetImage(swapchain_image).Create();
      swapchain_image_views_.emplace_back(std::move(swapchain_image_view));
    }

    ShaderModule::Creator shader_module_creator{ device_ };
    vert_shader_ = shader_module_creator.Load("C:\\workspace\\twopi\\src\\twopi\\shader\\vk\\triangle.vert.spv").Create();
    frag_shader_ = shader_module_creator.Load("C:\\workspace\\twopi\\src\\twopi\\shader\\vk\\triangle.frag.spv").Create();

    pipeline_layout_ = PipelineLayout::Creator{ device_ }.Create();

    render_pass_ = RenderPass::Creator{ device_ }
      .SetFormat(swapchain_images_[0])
      .Create();

    pipeline_ = GraphicsPipeline::Creator{ device_ }
      .SetShader(vert_shader_, frag_shader_)
      .SetVertexInput()
      .SetViewport(window->Width(), window->Height())
      .SetPipelineLayout(pipeline_layout_)
      .SetRenderPass(render_pass_)
      .Create();

    for (const auto& swapchain_image_view : swapchain_image_views_)
    {
      auto swapchain_framebuffer = Framebuffer::Creator{ device_ }
        .SetAttachment(swapchain_image_view)
        .SetExtent(window->Width(), window->Height())
        .SetRenderPass(render_pass_)
        .Create();

      swapchain_framebuffers_.emplace_back(std::move(swapchain_framebuffer));
    }

    command_pool_ = CommandPool::Creator{ device_ }
      .SetQueue(graphics_queue_)
      .Create();

    command_buffers_ = CommandBuffer::Allocator{ device_, command_pool_ }
      .Allocate(swapchain_framebuffers_.size());

    for (int i = 0; i < command_buffers_.size(); i++)
    {
      auto& command_buffer = command_buffers_[i];
      command_buffer
        .Begin()
        .BeginRenderPass(render_pass_, swapchain_framebuffers_[i])
        .BindPipeline(pipeline_)
        .Draw(3, 1, 0, 0)
        .EndRenderPass()
        .End();
    }

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

    for (auto& swapchain_framebuffer : swapchain_framebuffers_)
      swapchain_framebuffer.Destroy();
    swapchain_framebuffers_.clear();

    pipeline_.Destroy();
    pipeline_layout_.Destroy();
    render_pass_.Destroy();
    vert_shader_.Destroy();
    frag_shader_.Destroy();

    for (auto& swapchain_image_view : swapchain_image_views_)
      swapchain_image_view.Destroy();
    swapchain_image_views_.clear();

    swapchain_.Destroy();
    surface_.Destroy();
    device_.Destroy();
    instance_.Destroy();
  }

  void Draw()
  {
    in_flight_fences_[current_frame_].Wait();
    in_flight_fences_[current_frame_].Reset();

    const auto image_index = device_.AcquireNextImage(swapchain_, image_available_semaphores_[current_frame_]);

    if (images_in_flight_[image_index])
      images_in_flight_[image_index].Wait();
    images_in_flight_[image_index] = in_flight_fences_[current_frame_];

    in_flight_fences_[current_frame_].Reset();

    graphics_queue_.Submit(command_buffers_[image_index], { image_available_semaphores_[current_frame_] }, { render_finished_semaphores_[current_frame_] }, in_flight_fences_[current_frame_]);
    present_queue_.Present(swapchain_, image_index, { render_finished_semaphores_[current_frame_] });

    current_frame_ = (current_frame_ + 1) % max_frames_in_flight_;
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
}
}
