#include <twopi/gpu/gpu_engine.h>

#include <iostream>

#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <twopi/window/glfw_window.h>
#include <twopi/scene/camera.h>
#include <twopi/gpu/gpu_device.h>
#include <twopi/gpu/gpu_swapchain.h>

namespace twopi
{
namespace gpu
{
namespace
{
const std::string validation_layer_name = "VK_LAYER_KHRONOS_validation";

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
  VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
  VkDebugUtilsMessageTypeFlagsEXT message_type,
  const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
  void* pUserData)
{
  if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    std::cerr << callback_data->pMessage << std::endl << std::endl;

  return VK_FALSE;
}
}

class Engine::Impl
{
private:
  static constexpr int max_frames_in_flight_ = 2;

public:
  Impl(std::shared_ptr<window::Window> window)
  {
    width_ = window->Width();
    height_ = window->Height();

    vk::ApplicationInfo app_info{ "Twopi", VK_MAKE_VERSION(1, 0, 0), "Twopi Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_2 };

    std::vector<const char*> layer_names{
      "VK_LAYER_KHRONOS_validation"
    };

    std::vector<const char*> extension_names{
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    };
    uint32_t num_glfw_extensions;
    auto glfw_extensions = glfwGetRequiredInstanceExtensions(&num_glfw_extensions);
    for (uint32_t i = 0; i < num_glfw_extensions; i++)
      extension_names.push_back(glfw_extensions[i]);

    vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT> chain{
      { {}, &app_info, layer_names, extension_names },
      { {},
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        debug_callback
      },
    };
    instance_ = vk::createInstance(chain.get<vk::InstanceCreateInfo>());

    vk::DynamicLoader dl;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    vk::DispatchLoaderDynamic dld(instance_, vkGetInstanceProcAddr);
    messenger_ = instance_.createDebugUtilsMessengerEXT(chain.get<vk::DebugUtilsMessengerCreateInfoEXT>(), nullptr, dld);

    // Create surface
    VkSurfaceKHR surface;
    glfwCreateWindowSurface(instance_, std::static_pointer_cast<window::GlfwWindow>(window)->Handle(), nullptr, &surface);
    surface_ = surface;

    device_ = std::make_shared<Device>(instance_, surface_);
    const auto device_handle = device_->DeviceHandle();

    swapchain_ = std::make_shared<Swapchain>(device_, width_, height_);

    // Render pass
    const auto format = swapchain_->Format();
    vk::AttachmentDescription attachment{
      {}, format, vk::SampleCountFlagBits::e1,
      vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
      vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore,
      vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR
    };
    vk::AttachmentReference ref{ 0u, vk::ImageLayout::eColorAttachmentOptimal };
    vk::SubpassDescription subpass{ {}, vk::PipelineBindPoint::eGraphics, {}, ref, {} };
    vk::SubpassDependency subpass_dependency{
      VK_SUBPASS_EXTERNAL, 0u,
      vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
      vk::AccessFlags{}, vk::AccessFlagBits::eColorAttachmentWrite
    };
    render_pass_ = device_handle.createRenderPass({ {}, attachment, subpass, subpass_dependency });

    // Swapchain framebuffers
    const auto& image_views = swapchain_->ImageViews();
    for (int i = 0; i < image_views.size(); i++)
    {
      vk::ImageView image_view = image_views[i];
      swapchain_framebuffers_.emplace_back(device_handle.createFramebuffer({
        {}, render_pass_, image_view, width_, height_, 1u
        }));
    }

    // Triple buffering
    for (int i = 0; i < max_frames_in_flight_; i++)
    {
      image_available_semaphores_.emplace_back(device_handle.createSemaphore({}));
      render_finished_semaphores_.emplace_back(device_handle.createSemaphore({}));
    }
  }

  ~Impl()
  {
    const auto device_handle = device_->DeviceHandle();
    for (auto& semaphore : image_available_semaphores_)
      device_handle.destroySemaphore(semaphore);
    image_available_semaphores_.clear();

    for (auto& semaphore : render_finished_semaphores_)
      device_handle.destroySemaphore(semaphore);
    render_finished_semaphores_.clear();

    swapchain_ = nullptr;

    device_ = nullptr;

    instance_.destroySurfaceKHR(surface_);

    vk::DynamicLoader dl;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    vk::DispatchLoaderDynamic dld(instance_, vkGetInstanceProcAddr);
    instance_.destroyDebugUtilsMessengerEXT(messenger_, nullptr, dld);

    instance_.destroy();
  }

  void Draw()
  {
    const auto [image_index, result] = swapchain_->AcquireNextImage(image_available_semaphores_[current_frame_]);

    current_frame_ = (current_frame_ + 1) % max_frames_in_flight_;
  }

  void Resize(int width, int height)
  {
    swapchain_->Resize(width, height);
  }

  void UpdateCamera(std::shared_ptr<scene::Camera> camera)
  {
  }

private:
  vk::Instance instance_;
  vk::DebugUtilsMessengerEXT messenger_;
  vk::SurfaceKHR surface_;

  std::shared_ptr<Device> device_;
  std::shared_ptr<Swapchain> swapchain_;

  vk::RenderPass render_pass_;
  std::vector<vk::Framebuffer> swapchain_framebuffers_;

  std::vector<vk::Semaphore> image_available_semaphores_;
  std::vector<vk::Semaphore> render_finished_semaphores_;

  uint32_t width_ = 0;
  uint32_t height_ = 0;

  uint64_t current_frame_ = 0;
};

Engine::Engine(std::shared_ptr<window::Window> window)
  : impl_(std::make_unique<Impl>(window))
{
}

Engine::~Engine()
{
}

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
