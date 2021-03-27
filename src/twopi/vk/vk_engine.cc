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

namespace twopi
{
namespace vkw
{
class Engine::Impl
{
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
      const auto swapchain_framebuffer = Framebuffer::Creator{ device_ }
        .SetAttachment(swapchain_image_view)
        .SetExtent(window->Width(), window->Height())
        .SetRenderPass(render_pass_)
        .Create();

      swapchain_framebuffers_.emplace_back(std::move(swapchain_framebuffer));
    }
  }
  
  ~Impl()
  {
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
    // TODO
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
