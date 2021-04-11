#include <twopi/vkl/vkl_engine.h>

#include <iostream>
#include <optional>

#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <twopi/core/error.h>
#include <twopi/window/window.h>
#include <twopi/window/glfw_window.h>

namespace twopi
{
namespace vkl
{
namespace
{
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
  struct Memory
  {
    vk::DeviceMemory memory;
    uint32_t offset;
    uint32_t size;
  };

public:
  Impl() = delete;

  Impl(std::shared_ptr<window::Window> window)
  {
    glfw_window_handle_ = std::dynamic_pointer_cast<window::GlfwWindow>(window)->Handle();

    width_ = window->Width();
    height_ = window->Height();

    mip_levels_ = 3;

    Prepare();
  }

  ~Impl()
  {
    Cleanup();
  }

  void Draw(core::Duration duration)
  {
    std::cout << "Draw " << duration.count() << std::endl;
  }

  void Resize(int width, int height)
  {
    std::cout << "Resized: " << width << ' ' << height << std::endl;

    width_ = width;
    height_ = height;

    // TODO: Recreate swapchain
  }

  void UpdateLights(const std::vector<std::shared_ptr<scene::Light>>& lights)
  {
  }

  void UpdateCamera(std::shared_ptr<scene::Camera> camera)
  {
  }

private:
  void Prepare()
  {
    CreateInstance();
    CreateDevice();
    PreallocateMemories();
    CreateSwapchain();
  }

  void Cleanup()
  {
    CleanupSwapchain();
    FreePreallocatedMemories();
    CleanupDevice();
    CleanupInstance();
  }

  void CreateInstance()
  {
    // App
    vk::ApplicationInfo app_info;
    app_info
      .setPApplicationName("Twopi")
      .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
      .setPEngineName("Twopi Engine")
      .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
      .setApiVersion(VK_API_VERSION_1_2);

    // Layers
    std::vector<const char*> layers = {
      "VK_LAYER_KHRONOS_validation",
    };

    // Extensions
    std::vector<const char*> extensions = {
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    };

    uint32_t num_glfw_extensions = 0;
    const char** glfw_extensions;
    glfw_extensions = glfwGetRequiredInstanceExtensions(&num_glfw_extensions);
    for (uint32_t i = 0; i < num_glfw_extensions; i++)
      extensions.push_back(glfw_extensions[i]);

    // Messenger
    vk::DebugUtilsMessengerCreateInfoEXT messenger_create_info;
    messenger_create_info
      .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose)
      .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
      .setPfnUserCallback(debug_callback)
      .setPUserData(nullptr);

    // Create instance
    vk::InstanceCreateInfo instance_create_info;
    instance_create_info
      .setPApplicationInfo(&app_info)
      .setPEnabledLayerNames(layers)
      .setPEnabledExtensionNames(extensions)
      .setPNext(&messenger_create_info);

    instance_ = vk::createInstance(instance_create_info);

    // Create messneger
    vk::DynamicLoader dl;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    vk::DispatchLoaderDynamic dld(static_cast<vk::Instance>(instance_), vkGetInstanceProcAddr);
    messenger_ = instance_.createDebugUtilsMessengerEXT(messenger_create_info, nullptr, dld);

    // Create surface
    VkSurfaceKHR surface_handle;
    glfwCreateWindowSurface(instance_, glfw_window_handle_, nullptr, &surface_handle);
    surface_ = surface_handle;
  }

  void CleanupInstance()
  {
    instance_.destroySurfaceKHR(surface_);

    vk::DynamicLoader dl;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    vk::DispatchLoaderDynamic dld(instance_, vkGetInstanceProcAddr);
    instance_.destroyDebugUtilsMessengerEXT(messenger_, nullptr, dld);

    instance_.destroy();
  }

  void CreateDevice()
  {
    // Choose the first GPU
    physical_device_ = instance_.enumeratePhysicalDevices()[0];

    // Find queues
    const auto queue_family_properties = physical_device_.getQueueFamilyProperties();
    for (int i = 0; i < queue_family_properties.size(); i++)
    {
      if (!graphics_queue_index_ && queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
        graphics_queue_index_ = i;

      // TODO: select unique queue family indices
      if ((!present_queue_index_ || graphics_queue_index_ == present_queue_index_) && physical_device_.getSurfaceSupportKHR(i, surface_))
        present_queue_index_ = i;
    }

    float queue_priority = 1.f;
    vk::DeviceQueueCreateInfo graphics_queue_create_info;
    graphics_queue_create_info
      .setQueueFamilyIndex(graphics_queue_index_.value())
      .setQueueCount(1)
      .setQueuePriorities(queue_priority);

    vk::DeviceQueueCreateInfo present_queue_create_info;
    present_queue_create_info
      .setQueueFamilyIndex(present_queue_index_.value())
      .setQueueCount(1)
      .setQueuePriorities(queue_priority);

    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos = {
      graphics_queue_create_info,
      present_queue_create_info,
    };

    // Device extensions
    std::vector<const char*> extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    // Device features
    auto features = physical_device_.getFeatures();

    // Create device
    vk::DeviceCreateInfo device_create_info;
    device_create_info
      .setPEnabledExtensionNames(extensions)
      .setQueueCreateInfos(queue_create_infos)
      .setPEnabledFeatures(&features);

    device_ = physical_device_.createDevice(device_create_info);

    graphics_queue_ = device_.getQueue(graphics_queue_index_.value(), 0);
    present_queue_ = device_.getQueue(graphics_queue_index_.value(), 0);
  }

  void CleanupDevice()
  {
    device_.destroy();
  }

  void PreallocateMemories()
  {
    // Find memroy type index
    uint64_t device_available_size = 0;
    int device_index = 0;
    uint64_t host_available_size = 0;
    int host_index = 0;
    const auto memory_properties = physical_device_.getMemoryProperties();
    for (int i = 0; i < memory_properties.memoryTypeCount; i++)
    {
      const auto properties = memory_properties.memoryTypes[i].propertyFlags;
      const auto heap_index = memory_properties.memoryTypes[i].heapIndex;
      const auto heap = memory_properties.memoryHeaps[heap_index];

      if ((properties & vk::MemoryPropertyFlagBits::eDeviceLocal) == vk::MemoryPropertyFlagBits::eDeviceLocal)
      {
        if (heap.size > device_available_size)
        {
          device_index = i;
          device_available_size = heap.size;
        }
      }

      if ((properties & (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent))
        == (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent))
      {
        if (heap.size > host_available_size)
        {
          host_index = i;
          host_available_size = heap.size;
        }
      }
    }

    constexpr uint64_t chunk_size = 256 * 1024 * 1024; // 256MB

    vk::MemoryAllocateInfo allocate_info;
    allocate_info
      .setAllocationSize(chunk_size)
      .setMemoryTypeIndex(device_index);
    device_memory_ = device_.allocateMemory(allocate_info);

    allocate_info
      .setMemoryTypeIndex(host_index);
    host_memory_ = device_.allocateMemory(allocate_info);
  }

  void FreePreallocatedMemories()
  {
    device_.freeMemory(device_memory_);
    device_.freeMemory(host_memory_);
  }

  void CreateSwapchain()
  {
    const auto capabilities = physical_device_.getSurfaceCapabilitiesKHR(surface_);

    // Triple buffering
    auto image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
      image_count = capabilities.maxImageCount;

    if (image_count != 3)
      throw core::Error("Triple buffering is not supported.");

    vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo;
    const auto present_modes = physical_device_.getSurfacePresentModesKHR(surface_);
    for (auto available_mode : present_modes)
    {
      if (available_mode == vk::PresentModeKHR::eMailbox)
        present_mode = vk::PresentModeKHR::eMailbox;
    }

    // Format
    const auto available_formats = physical_device_.getSurfaceFormatsKHR(surface_);
    auto format = available_formats[0];
    for (const auto& available_format : available_formats)
    {
      if (available_format.format == vk::Format::eB8G8R8A8Srgb &&
        available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        format = available_format;
    }

    // Extent
    vk::Extent2D extent;
    if (capabilities.currentExtent.width != UINT32_MAX)
      extent = capabilities.currentExtent;
    else
    {
      VkExtent2D actual_extent = { width_, height_ };

      actual_extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actual_extent.width));
      actual_extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actual_extent.height));

      extent = actual_extent;
    }

    // Image sharing mode
    std::vector<uint32_t> queue_family_indices{
      graphics_queue_index_.value(),
      present_queue_index_.value(),
    };

    // Create swapchain
    vk::SwapchainCreateInfoKHR swapchain_create_info;
    swapchain_create_info
      .setSurface(surface_)
      .setImageArrayLayers(1)
      .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
      .setPreTransform(capabilities.currentTransform)
      .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
      .setClipped(VK_TRUE)
      .setOldSwapchain(nullptr)
      .setMinImageCount(image_count)
      .setPresentMode(present_mode)
      .setImageFormat(format.format)
      .setImageColorSpace(format.colorSpace)
      .setImageExtent(extent)
      .setImageSharingMode(vk::SharingMode::eConcurrent)
      .setQueueFamilyIndices(queue_family_indices);

    swapchain_ = device_.createSwapchainKHR(swapchain_create_info);
    swapchain_image_format_ = format.format;

    swapchain_images_ = device_.getSwapchainImagesKHR(swapchain_);

    // Create image view for swapchain
    vk::ImageSubresourceRange image_subresource_range;
    image_subresource_range
      .setAspectMask(vk::ImageAspectFlagBits::eColor)
      .setLevelCount(1)
      .setBaseMipLevel(0)
      .setLayerCount(1)
      .setBaseArrayLayer(0);

    vk::ImageViewCreateInfo image_view_create_info;
    image_view_create_info
      .setViewType(vk::ImageViewType::e2D)
      .setComponents(vk::ComponentMapping{})
      .setFormat(swapchain_image_format_)
      .setSubresourceRange(image_subresource_range);

    swapchain_image_views_.resize(swapchain_images_.size());
    for (int i = 0; i < swapchain_images_.size(); i++)
    {
      image_view_create_info
        .setImage(swapchain_images_[i]);

      swapchain_image_views_[i] = device_.createImageView(image_view_create_info);
    }

    // Create multisample rendertarget image
    vk::ImageCreateInfo image_create_info;
    image_create_info
      .setImageType(vk::ImageType::e2D)
      .setMipLevels(1)
      .setArrayLayers(1)
      .setTiling(vk::ImageTiling::eOptimal)
      .setInitialLayout(vk::ImageLayout::eUndefined)
      .setSharingMode(vk::SharingMode::eExclusive)
      .setSamples(vk::SampleCountFlagBits::e4)
      .setExtent(vk::Extent3D{ extent.width, extent.height, 1 })
      .setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment)
      .setFormat(swapchain_image_format_);
    rendertarget_image_ = device_.createImage(image_create_info);

    rendertarget_image_memory_ = AllocateDeviceMemory(rendertarget_image_);
    device_.bindImageMemory(rendertarget_image_, rendertarget_image_memory_.memory, rendertarget_image_memory_.offset);

    image_view_create_info
      .setFormat(swapchain_image_format_)
      .setImage(rendertarget_image_);
    rendertarget_image_view_ = device_.createImageView(image_view_create_info);

    // Create multisample depth image
    image_create_info
      .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
      .setFormat(vk::Format::eD24UnormS8Uint);
    depth_image_ = device_.createImage(image_create_info);

    depth_image_memory_ = AllocateDeviceMemory(depth_image_);
    device_.bindImageMemory(depth_image_, depth_image_memory_.memory, depth_image_memory_.offset);

    image_subresource_range
      .setAspectMask(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil);

    image_view_create_info
      .setSubresourceRange(image_subresource_range)
      .setFormat(vk::Format::eD24UnormS8Uint)
      .setImage(depth_image_);
    depth_image_view_ = device_.createImageView(image_view_create_info);
  }

  void CleanupSwapchain()
  {
    for (auto& swapchain_iamge_view : swapchain_image_views_)
      device_.destroyImageView(swapchain_iamge_view);
    swapchain_image_views_.clear();

    device_.destroyImage(depth_image_);
    device_.destroyImageView(depth_image_view_);

    device_.destroyImage(rendertarget_image_);
    device_.destroyImageView(rendertarget_image_view_);

    device_.destroySwapchainKHR(swapchain_);
  }

  void CreateRenderPass()
  {
    // Create render pass
    vk::AttachmentDescription color_attachment;
    color_attachment
      .setSamples(vk::SampleCountFlagBits::e4)
      .setLoadOp(vk::AttachmentLoadOp::eClear)
      .setStoreOp(vk::AttachmentStoreOp::eStore)
      .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
      .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
      .setInitialLayout(vk::ImageLayout::eUndefined)
      .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal)
      .setFormat(swapchain_image_format_);

    vk::AttachmentReference color_attachment_ref;
    color_attachment_ref
      .setAttachment(0)
      .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::AttachmentDescription depth_attachment;
    depth_attachment
      .setSamples(vk::SampleCountFlagBits::e4)
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
      .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

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

    swapchain_render_pass_ = device_.createRenderPass(render_pass_create_info);
  }

  void CleanupRenderPass()
  {
    device_.destroyRenderPass(swapchain_render_pass_);
  }

  void CreateSwapchainFramebuffers()
  {
    // Create framebuffers
    vk::FramebufferCreateInfo framebuffer_create_info;
    framebuffer_create_info
      .setWidth(width_)
      .setHeight(height_)
      .setLayers(1)
      .setRenderPass(swapchain_render_pass_);

    swapchain_framebuffers_.resize(swapchain_images_.size());
    for (int i = 0; i < swapchain_images_.size(); i++)
    {
      const std::vector<vk::ImageView> attachments = {
        rendertarget_image_view_,
        depth_image_view_,
        swapchain_image_views_[i],
      };

      framebuffer_create_info
        .setAttachments(attachments);

      swapchain_framebuffers_[i] = device_.createFramebuffer(framebuffer_create_info);
    }
  }

  void CleanupSwapchainFramebuffers()
  {
    for (auto& swapchain_framebuffer : swapchain_framebuffers_)
      device_.destroyFramebuffer(swapchain_framebuffer);
    swapchain_framebuffers_.clear();
  }

  void CreateSampler()
  {
    vk::SamplerCreateInfo sampler_create_info;
    sampler_create_info
      .setAnisotropyEnable(true)
      .setMaxAnisotropy(physical_device_.getProperties().limits.maxSamplerAnisotropy)
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
      .setMaxLod(mip_levels_);

    sampler_ = device_.createSampler(sampler_create_info);
  }

  void CleanupSampler()
  {
    device_.destroySampler(sampler_);
  }

  Memory AllocateDeviceMemory(vk::Image image)
  {
    return AllocateDeviceMemory(device_.getImageMemoryRequirements(image));
  }

  Memory AllocateDeviceMemory(vk::Buffer buffer)
  {
    return AllocateDeviceMemory(device_.getBufferMemoryRequirements(buffer));
  }

  Memory AllocateDeviceMemory(const vk::MemoryRequirements& requirements)
  {
    Memory memory;
    memory.memory = device_memory_;
    memory.offset = (device_memory_offset_ + requirements.alignment - 1) / requirements.alignment * requirements.alignment;
    memory.size = requirements.size;
    device_memory_offset_ = memory.offset + memory.size;
    return memory;
  }

  Memory AllocateHostMemory(vk::Image image)
  {
    return AllocateHostMemory(device_.getImageMemoryRequirements(image));
  }

  Memory AllocateHostMemory(vk::Buffer buffer)
  {
    return AllocateHostMemory(device_.getBufferMemoryRequirements(buffer));
  }

  Memory AllocateHostMemory(const vk::MemoryRequirements& requirements)
  {
    Memory memory;
    memory.memory = host_memory_;
    memory.offset = (host_memory_offset_ + requirements.alignment - 1) / requirements.alignment * requirements.alignment;
    memory.size = requirements.size;
    host_memory_offset_ = memory.offset + memory.size;
    return memory;
  }

  GLFWwindow* glfw_window_handle_;

  // Instance
  vk::Instance instance_;
  vk::DebugUtilsMessengerEXT messenger_;
  vk::SurfaceKHR surface_;

  // Device
  vk::PhysicalDevice physical_device_;
  vk::Device device_;
  std::optional<uint32_t> graphics_queue_index_ = 0;
  std::optional<uint32_t> present_queue_index_ = 0;
  vk::Queue graphics_queue_;
  vk::Queue present_queue_;

  // Memory
  vk::DeviceMemory device_memory_;
  vk::DeviceMemory host_memory_;
  uint64_t device_memory_offset_ = 0;
  uint64_t host_memory_offset_ = 0;

  // Swapchain
  vk::SwapchainKHR swapchain_;
  vk::Format swapchain_image_format_;
  uint32_t width_ = 0;
  uint32_t height_ = 0;
  std::vector<vk::Image> swapchain_images_;
  std::vector<vk::ImageView> swapchain_image_views_;
  vk::Image rendertarget_image_;
  Memory rendertarget_image_memory_;
  vk::ImageView rendertarget_image_view_;
  vk::Image depth_image_;
  Memory depth_image_memory_;
  vk::ImageView depth_image_view_;

  // Swapchain render pass
  vk::RenderPass swapchain_render_pass_;

  // Swapchain framebuffers
  std::vector<vk::Framebuffer> swapchain_framebuffers_;

  // Sampler
  uint32_t mip_levels_ = 1;
  vk::Sampler sampler_;
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
}
}
