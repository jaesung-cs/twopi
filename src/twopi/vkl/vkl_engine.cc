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
    uint64_t offset = 0;
    uint64_t size = 0;
  };

  struct Buffer
  {
    vk::Buffer buffer;
    Memory memory;
  };

  struct Mesh
  {
    Buffer buffer;
    uint64_t position_offset;
    uint64_t normal_offset;
    uint64_t tex_coord_offset;
    uint64_t index_offset;
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

  struct DynamicUniformBuffer
  {
    Buffer buffer;
    uint32_t stride = 0;
  };

  struct UniformBuffer
  {
    Buffer buffer;
    uint32_t stride = 0;
  };

public:
  Impl() = delete;

  Impl(std::shared_ptr<window::Window> window)
  {
    glfw_window_handle_ = std::dynamic_pointer_cast<window::GlfwWindow>(window)->Handle();

    width_ = window->Width();
    height_ = window->Height();

    mip_levels_ = 3;

    num_objects_ = 3; // One floor, one object, one light

    material_.specular = glm::vec3(1.f, 1.f, 1.f);
    material_.shininess = 64.f;

    floor_model_.model = glm::mat4(1.f);
    floor_model_.model_inverse_transpose = glm::inverse(glm::transpose(floor_model_.model));

    mesh_model_.model = glm::rotate(glm::pi<float>() / 2.f, glm::vec3(1.f, 0.f, 0.f));
    mesh_model_.model[3][2] = 1.f;
    mesh_model_.model_inverse_transpose = glm::inverse(glm::transpose(mesh_model_.model));

    Prepare();
  }

  ~Impl()
  {
    Cleanup();
  }

  void Draw(core::Duration duration)
  {
    device_.waitForFences(in_flight_fences_[current_frame_], true, UINT64_MAX);

    const auto result = device_.acquireNextImageKHR(swapchain_, UINT64_MAX, image_available_semaphores_[current_frame_]);
    if (result.result != vk::Result::eSuccess && result.result != vk::Result::eSuboptimalKHR)
      throw core::Error("Failed to acquire next swapchain image.");

    const auto& image_index = result.value;

    if (images_in_flight_[image_index])
      device_.waitForFences(images_in_flight_[image_index], true, UINT64_MAX);
    images_in_flight_[image_index] = in_flight_fences_[current_frame_];

    device_.resetFences(in_flight_fences_[current_frame_]);

    // Update uniforms
    std::memcpy(uniform_buffer_map_ + camera_ubos_[image_index].buffer.memory.offset, &camera_, sizeof(CameraUbo));
    std::memcpy(uniform_buffer_map_ + light_ubos_[image_index].buffer.memory.offset, &lights_, sizeof(LightUbo));
    std::memcpy(dynamic_uniform_buffer_map_ + model_ubos_[image_index].buffer.memory.offset, &floor_model_, sizeof(ModelUbo));
    std::memcpy(dynamic_uniform_buffer_map_ + model_ubos_[image_index].buffer.memory.offset + model_ubos_[image_index].stride, &light_model_, sizeof(ModelUbo));
    std::memcpy(dynamic_uniform_buffer_map_ + model_ubos_[image_index].buffer.memory.offset + model_ubos_[image_index].stride * 2, &mesh_model_, sizeof(ModelUbo));
    std::memcpy(dynamic_uniform_buffer_map_ + material_ubos_[image_index].buffer.memory.offset, &material_, sizeof(MaterialUbo));

    std::vector<vk::PipelineStageFlags> stage_mask = {
      vk::PipelineStageFlagBits::eColorAttachmentOutput,
    };
    vk::SubmitInfo submit_info;
    submit_info
      .setWaitSemaphores(image_available_semaphores_[current_frame_])
      .setCommandBuffers(draw_command_buffers_[image_index])
      .setSignalSemaphores(render_finished_semaphores_[current_frame_])
      .setWaitDstStageMask(stage_mask);
    graphics_queue_.submit(submit_info, in_flight_fences_[current_frame_]);

    std::vector<uint32_t> image_indices{ image_index };
    vk::PresentInfoKHR present_info;
    present_info
      .setSwapchains(swapchain_)
      .setWaitSemaphores(render_finished_semaphores_[current_frame_])
      .setImageIndices(image_indices);
    const auto present_result = present_queue_.presentKHR(present_info);

    if (present_result == vk::Result::eErrorOutOfDateKHR || present_result == vk::Result::eSuboptimalKHR)
      throw core::Error("Need to recreate swapchain.");
    else if (present_result != vk::Result::eSuccess)
      throw core::Error("Failed to present swapchain image.");

    current_frame_ = (current_frame_ + 1) % max_frames_in_flight_;
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

private:
  void Prepare()
  {
    CreateInstance();
    CreateDevice();
    PreallocateMemories();
    CreateSwapchain();
    CreateRenderPass();
    CreateSwapchainFramebuffers();
    CreateSampler();
    CreateDescriptorSet();
    CreateGraphicsPipelines();
    CreateCommandPool();
    CreateSynchronizationObjects();
    PrepareResources();
    CreateCommandBuffers();
  }

  void Cleanup()
  {
    device_.waitIdle();

    DestroyCommandBuffers();
    CleanupResources();
    DestroySynchronizationObjects();
    DestroyCommandPool();
    DestroyGraphicsPipelines();
    DestroyDesciptorSet();
    DestroySampler();
    DestroySwapchainFramebuffers();
    DestroyRenderPass();
    DestroySwapchain();
    FreePreallocatedMemories();
    DestroyDevice();
    DestroyInstance();
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

  void DestroyInstance()
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

  void DestroyDevice()
  {
    device_.destroy();
  }

  void PreallocateMemories()
  {
    // Find memroy type index
    uint64_t device_available_size = 0;
    uint64_t host_available_size = 0;
    const auto memory_properties = physical_device_.getMemoryProperties();
    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
    {
      const auto properties = memory_properties.memoryTypes[i].propertyFlags;
      const auto heap_index = memory_properties.memoryTypes[i].heapIndex;
      const auto heap = memory_properties.memoryHeaps[heap_index];

      if ((properties & vk::MemoryPropertyFlagBits::eDeviceLocal) == vk::MemoryPropertyFlagBits::eDeviceLocal)
      {
        if (heap.size > device_available_size)
        {
          device_index_ = i;
          device_available_size = heap.size;
        }
      }

      if ((properties & (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent))
        == (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent))
      {
        if (heap.size > host_available_size)
        {
          host_index_ = i;
          host_available_size = heap.size;
        }
      }
    }

    constexpr uint64_t chunk_size = 256 * 1024 * 1024; // 256MB

    vk::MemoryAllocateInfo allocate_info;
    allocate_info
      .setAllocationSize(chunk_size)
      .setMemoryTypeIndex(device_index_);
    device_memory_ = device_.allocateMemory(allocate_info);

    allocate_info
      .setMemoryTypeIndex(host_index_);
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

  void DestroySwapchain()
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
      .setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
      .setFormat(swapchain_image_format_);

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

  void DestroyRenderPass()
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

  void DestroySwapchainFramebuffers()
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
      .setMaxLod(static_cast<float>(mip_levels_));

    sampler_ = device_.createSampler(sampler_create_info);
  }

  void DestroySampler()
  {
    device_.destroySampler(sampler_);
  }

  void CreateDescriptorSet()
  {
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    vk::DescriptorSetLayoutBinding binding;

    // Binding 0: CameraUbo
    binding
      .setBinding(0)
      .setDescriptorType(vk::DescriptorType::eUniformBuffer)
      .setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
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

    descriptor_set_layout_ = device_.createDescriptorSetLayout(descriptor_set_layout_create_info);

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
    descriptor_pool_ = device_.createDescriptorPool(descriptor_pool_create_info);
  }

  void DestroyDesciptorSet()
  {
    device_.destroyDescriptorPool(descriptor_pool_);
    device_.destroyDescriptorSetLayout(descriptor_set_layout_);
  }

  void CreateGraphicsPipelines()
  {
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

    pipeline_layout_ = device_.createPipelineLayout(pipeline_layout_create_info);

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
      .setRenderPass(swapchain_render_pass_)
      .setSubpass(0)
      .setPVertexInputState(&vertex_input_info)
      .setPInputAssemblyState(&input_assembly_info)
      .setPViewportState(&viewport_state_info)
      .setPRasterizationState(&rasterization_info)
      .setPMultisampleState(&multisample_info)
      .setPDepthStencilState(nullptr)
      .setPColorBlendState(&color_blend_info)
      .setPDepthStencilState(&depth_stencil_info)
      .setPDynamicState(nullptr) // TODO
      .setBasePipelineHandle(nullptr)
      .setBasePipelineIndex(-1)
      .setStages(shader_stages);

    color_pipeline_ = device_.createGraphicsPipeline(nullptr, graphics_pipeline_create_info).value;

    device_.destroyShaderModule(vert_shader_module);
    device_.destroyShaderModule(frag_shader_module);
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
      .setPName("main")
      .setModule(frag_shader_module);
    shader_stages.push_back(shader_stage);

    graphics_pipeline_create_info
      .setStages(shader_stages);

    floor_pipeline_ = device_.createGraphicsPipeline(nullptr, graphics_pipeline_create_info).value;

    device_.destroyShaderModule(vert_shader_module);
    device_.destroyShaderModule(frag_shader_module);
    shader_stages.clear();
  }

  void DestroyGraphicsPipelines()
  {
    device_.destroyPipelineLayout(pipeline_layout_);
    device_.destroyPipeline(color_pipeline_);
    device_.destroyPipeline(floor_pipeline_);
  }

  void CreateCommandPool()
  {
    vk::CommandPoolCreateInfo command_pool_create_info;
    command_pool_create_info
      .setQueueFamilyIndex(graphics_queue_index_.value());

    command_pool_ = device_.createCommandPool(command_pool_create_info);

    command_pool_create_info
      .setFlags(vk::CommandPoolCreateFlagBits::eTransient);

    transient_command_pool_ = device_.createCommandPool(command_pool_create_info);
  }

  void DestroyCommandPool()
  {
    device_.destroyCommandPool(command_pool_);
    device_.destroyCommandPool(transient_command_pool_);
  }

  void CreateSynchronizationObjects()
  {
    vk::SemaphoreCreateInfo semaphore_create_info;
    vk::FenceCreateInfo fence_create_info;

    transfer_fence_ = device_.createFence(fence_create_info);

    fence_create_info
      .setFlags(vk::FenceCreateFlagBits::eSignaled);

    for (int i = 0; i < swapchain_images_.size(); i++)
    {
      image_available_semaphores_.emplace_back(device_.createSemaphore(semaphore_create_info));
      render_finished_semaphores_.emplace_back(device_.createSemaphore(semaphore_create_info));
      in_flight_fences_.emplace_back(device_.createFence(fence_create_info));
    }

    images_in_flight_.resize(swapchain_images_.size());
  }

  void DestroySynchronizationObjects()
  {
    for (auto& semaphore : image_available_semaphores_)
      device_.destroySemaphore(semaphore);
    image_available_semaphores_.clear();

    for (auto& semaphore : render_finished_semaphores_)
      device_.destroySemaphore(semaphore);
    render_finished_semaphores_.clear();

    for (auto& fence : in_flight_fences_)
      device_.destroyFence(fence);
    in_flight_fences_.clear();

    device_.destroyFence(transfer_fence_);
  }

  void PrepareResources()
  {
    // Dynamic ubo alignment
    const auto dynamic_ubo_alignment = physical_device_.getProperties().limits.minUniformBufferOffsetAlignment;

    const auto camera_stride = (sizeof(CameraUbo) + dynamic_ubo_alignment - 1) & ~(dynamic_ubo_alignment - 1);
    const auto light_stride = (sizeof(LightUbo) + dynamic_ubo_alignment - 1) & ~(dynamic_ubo_alignment - 1);

    // Uniform buffers
    vk::BufferCreateInfo buffer_create_info;
    buffer_create_info
      .setSharingMode(vk::SharingMode::eExclusive)
      .setUsage(vk::BufferUsageFlagBits::eUniformBuffer)
      .setSize((camera_stride + light_stride) * swapchain_images_.size());
    uniform_buffer_.buffer = device_.createBuffer(buffer_create_info);
    uniform_buffer_.memory = AllocatePersistentlyMappedMemory(uniform_buffer_.buffer);
    device_.bindBufferMemory(uniform_buffer_.buffer, uniform_buffer_.memory.memory, uniform_buffer_.memory.offset);
    uniform_buffer_map_ = static_cast<unsigned char*>(device_.mapMemory(uniform_buffer_.memory.memory, uniform_buffer_.memory.offset, uniform_buffer_.memory.size));

    camera_ubos_.resize(swapchain_images_.size());
    for (int i = 0; i < camera_ubos_.size(); i++)
    {
      Buffer buffer;
      buffer.buffer = uniform_buffer_.buffer;
      buffer.memory.memory = uniform_buffer_.memory.memory;
      buffer.memory.offset = uniform_buffer_.memory.offset + camera_stride * i;
      buffer.memory.size = sizeof(CameraUbo);

      camera_ubos_[i].buffer = buffer;
    }

    light_ubos_.resize(swapchain_images_.size());
    for (int i = 0; i < light_ubos_.size(); i++)
    {
      Buffer buffer;
      buffer.buffer = uniform_buffer_.buffer;
      buffer.memory.memory = uniform_buffer_.memory.memory;
      buffer.memory.offset = uniform_buffer_.memory.offset + camera_stride * camera_ubos_.size() + light_stride * i;
      buffer.memory.size = sizeof(LightUbo);

      light_ubos_[i].buffer = buffer;
    }

    // Dynamic uniform buffers
    const auto model_stride = (sizeof(ModelUbo) + dynamic_ubo_alignment - 1) & ~(dynamic_ubo_alignment - 1);
    const auto material_stride = (sizeof(MaterialUbo) + dynamic_ubo_alignment - 1) & ~(dynamic_ubo_alignment - 1);

    buffer_create_info
      .setSize((model_stride + material_stride) * num_objects_ * swapchain_images_.size());
    dynamic_uniform_buffer_.buffer = device_.createBuffer(buffer_create_info);
    dynamic_uniform_buffer_.memory = AllocatePersistentlyMappedMemory(dynamic_uniform_buffer_.buffer);
    device_.bindBufferMemory(dynamic_uniform_buffer_.buffer, dynamic_uniform_buffer_.memory.memory, dynamic_uniform_buffer_.memory.offset);
    dynamic_uniform_buffer_map_ = static_cast<unsigned char*>(device_.mapMemory(dynamic_uniform_buffer_.memory.memory, dynamic_uniform_buffer_.memory.offset, dynamic_uniform_buffer_.memory.size));

    model_ubos_.resize(swapchain_images_.size());
    for (int i = 0; i < model_ubos_.size(); i++)
    {
      Buffer buffer;
      buffer.buffer = dynamic_uniform_buffer_.buffer;
      buffer.memory.memory = dynamic_uniform_buffer_.memory.memory;
      buffer.memory.offset = dynamic_uniform_buffer_.memory.offset + model_stride * i;
      buffer.memory.size = sizeof(ModelUbo);

      model_ubos_[i].buffer = buffer;
      model_ubos_[i].stride = model_stride;
    }

    material_ubos_.resize(swapchain_images_.size());
    for (int i = 0; i < material_ubos_.size(); i++)
    {
      Buffer buffer;
      buffer.buffer = dynamic_uniform_buffer_.buffer;
      buffer.memory.memory = dynamic_uniform_buffer_.memory.memory;
      buffer.memory.offset = dynamic_uniform_buffer_.memory.offset + model_stride * num_objects_ * swapchain_images_.size() + material_stride * i;
      buffer.memory.size = sizeof(MaterialUbo);

      material_ubos_[i].buffer = buffer;
      material_ubos_[i].stride = material_stride;
    }

    // Allocate descriptor sets
    std::vector<vk::DescriptorSetLayout> layouts(swapchain_images_.size(), descriptor_set_layout_);
    vk::DescriptorSetAllocateInfo descriptor_set_allocate_info;
    descriptor_set_allocate_info
      .setSetLayouts(layouts)
      .setDescriptorPool(descriptor_pool_);

    descriptor_sets_ = device_.allocateDescriptorSets(descriptor_set_allocate_info);

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

    // TODO
    /*
    write
      .setDstBinding(4)
      .setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    writes.push_back(write);
    */

    for (int i = 0; i < swapchain_images_.size(); i++)
    {
      buffer_infos[0]
        .setBuffer(camera_ubos_[i].buffer.buffer)
        .setOffset(camera_ubos_[i].buffer.memory.offset)
        .setRange(camera_stride);

      buffer_infos[1]
        .setBuffer(model_ubos_[i].buffer.buffer)
        .setOffset(model_ubos_[i].buffer.memory.offset)
        .setRange(model_ubos_[i].stride);

      buffer_infos[2]
        .setBuffer(light_ubos_[i].buffer.buffer)
        .setOffset(light_ubos_[i].buffer.memory.offset)
        .setRange(light_stride);

      buffer_infos[3]
        .setBuffer(material_ubos_[i].buffer.buffer)
        .setOffset(material_ubos_[i].buffer.memory.offset)
        .setRange(material_ubos_[i].stride);

      writes[0].setBufferInfo(buffer_infos[0]);
      writes[1].setBufferInfo(buffer_infos[1]);
      writes[2].setBufferInfo(buffer_infos[2]);
      writes[3].setBufferInfo(buffer_infos[3]);

      // TODO: image
      /*
      image_infos[0];
      writes[4].setImageInfo(image_infos[0]);
      */

      for (auto& write : writes)
        write.setDstSet(descriptor_sets_[i]);

      device_.updateDescriptorSets(writes, nullptr);
    }

    // Stage buffer
    buffer_create_info
      .setSharingMode(vk::SharingMode::eExclusive)
      .setUsage(vk::BufferUsageFlagBits::eTransferSrc)
      .setSize(32 * 1024 * 1024); // 32MB
    stage_buffer_.buffer = device_.createBuffer(buffer_create_info);
    stage_buffer_.memory = AllocateHostMemory(stage_buffer_.buffer);
    device_.bindBufferMemory(stage_buffer_.buffer, stage_buffer_.memory.memory, stage_buffer_.memory.offset);

    // Floor
    constexpr float floor_range = 30.f;

    std::vector<float> floor_buffer = {
      // position
      -floor_range, -floor_range, 0.f,
      floor_range, -floor_range, 0.f,
      -floor_range, floor_range, 0.f,
      floor_range, floor_range, 0.f,
      // normal
      0.f, 0.f, 1.f,
      0.f, 0.f, 1.f,
      0.f, 0.f, 1.f,
      0.f, 0.f, 1.f,
      // tex_coord
      -floor_range, -floor_range,
      floor_range, -floor_range,
      -floor_range, floor_range,
      floor_range, floor_range,
    };

    std::vector<uint32_t> floor_indices = {
      0, 1, 2, 2, 1, 3
    };

    floor_position_offset_ = 0;
    floor_normal_offset_ = sizeof(float) * 12;
    floor_tex_coord_offset_ = sizeof(float) * 24;
    floor_index_offset_ = sizeof(float) * 32;
    floor_num_indices_ = static_cast<uint32_t>(floor_indices.size());

    const uint64_t floor_buffer_size = floor_buffer.size() * sizeof(float) + floor_indices.size() * sizeof(uint32_t);

    auto* ptr = static_cast<unsigned char*>(device_.mapMemory(stage_buffer_.memory.memory, stage_buffer_.memory.offset, floor_buffer_size));
    std::memcpy(ptr, floor_buffer.data(), floor_buffer.size() * sizeof(float));
    std::memcpy(ptr + floor_buffer.size() * sizeof(float), floor_indices.data(), floor_indices.size() * sizeof(uint32_t));
    device_.unmapMemory(stage_buffer_.memory.memory);

    buffer_create_info
      .setUsage(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer)
      .setSize(floor_buffer_size);
    floor_buffer_.buffer = device_.createBuffer(buffer_create_info);
    floor_buffer_.memory = AllocateDeviceMemory(floor_buffer_.buffer);
    device_.bindBufferMemory(floor_buffer_.buffer, floor_buffer_.memory.memory, floor_buffer_.memory.offset);

    vk::CommandBufferAllocateInfo allocate_info;
    allocate_info
      .setLevel(vk::CommandBufferLevel::ePrimary)
      .setCommandPool(transient_command_pool_)
      .setCommandBufferCount(3);
    auto transfer_commands = device_.allocateCommandBuffers(allocate_info);

    vk::CommandBufferBeginInfo begin_info;
    begin_info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    transfer_commands[0].begin(begin_info);

    vk::BufferCopy region;
    region
      .setSrcOffset(0)
      .setDstOffset(0)
      .setSize(floor_buffer_size);
    transfer_commands[0].copyBuffer(stage_buffer_.buffer, floor_buffer_.buffer, region);
    transfer_commands[0].end();

    // Sphere
    constexpr int sphere_grid_size = 32;

    std::vector<float> sphere_vertex_buffer;
    std::vector<float> sphere_normal_buffer;
    std::vector<uint32_t> sphere_index_buffer;
    const uint32_t top_index = sphere_grid_size * (sphere_grid_size - 1);
    const uint32_t bottom_index = top_index + 1;
    for (int i = 0; i < sphere_grid_size; i++)
    {
      sphere_index_buffer.push_back(top_index);
      sphere_index_buffer.push_back(i * (sphere_grid_size - 1));
      sphere_index_buffer.push_back(((i + 1) % sphere_grid_size) * (sphere_grid_size - 1));

      const float u = static_cast<float>(i) / sphere_grid_size;
      const float theta = u * 2.f * glm::pi<float>();
      for (int j = 1; j < sphere_grid_size; j++)
      {
        const float v = static_cast<float>(j) / sphere_grid_size;
        const float phi = v * glm::pi<float>();
        sphere_vertex_buffer.push_back(std::cos(theta) * std::sin(phi));
        sphere_vertex_buffer.push_back(std::sin(theta) * std::sin(phi));
        sphere_vertex_buffer.push_back(std::cos(phi));
        sphere_normal_buffer.push_back(std::cos(theta) * std::sin(phi));
        sphere_normal_buffer.push_back(std::sin(theta) * std::sin(phi));
        sphere_normal_buffer.push_back(std::cos(phi));

        if (j < sphere_grid_size - 1)
        {
          sphere_index_buffer.push_back(i * (sphere_grid_size - 1) + j - 1);
          sphere_index_buffer.push_back(i * (sphere_grid_size - 1) + j);
          sphere_index_buffer.push_back(((i + 1) % sphere_grid_size) * (sphere_grid_size - 1) + j - 1);

          sphere_index_buffer.push_back(i * (sphere_grid_size - 1) + j);
          sphere_index_buffer.push_back(((i + 1) % sphere_grid_size) * (sphere_grid_size - 1) + j);
          sphere_index_buffer.push_back(((i + 1) % sphere_grid_size) * (sphere_grid_size - 1) + j - 1);
        }
      }

      sphere_index_buffer.push_back(i * (sphere_grid_size - 1) + sphere_grid_size - 2);
      sphere_index_buffer.push_back(bottom_index);
      sphere_index_buffer.push_back(((i + 1) % sphere_grid_size) * (sphere_grid_size - 1) + sphere_grid_size - 2);
    }
    sphere_vertex_buffer.push_back(0.f);
    sphere_vertex_buffer.push_back(0.f);
    sphere_vertex_buffer.push_back(1.f);
    sphere_normal_buffer.push_back(0.f);
    sphere_normal_buffer.push_back(0.f);
    sphere_normal_buffer.push_back(1.f);

    sphere_vertex_buffer.push_back(0.f);
    sphere_vertex_buffer.push_back(0.f);
    sphere_vertex_buffer.push_back(-1.f);
    sphere_normal_buffer.push_back(0.f);
    sphere_normal_buffer.push_back(0.f);
    sphere_normal_buffer.push_back(-1.f);

    sphere_position_offset_ = 0;
    sphere_normal_offset_ = static_cast<uint32_t>(sphere_vertex_buffer.size()) * sizeof(float);
    sphere_index_offset_ = static_cast<uint32_t>(sphere_vertex_buffer.size() + sphere_normal_buffer.size()) * sizeof(float);
    sphere_num_indices_ = static_cast<uint32_t>(sphere_index_buffer.size());

    const auto sphere_buffer_size = (sphere_vertex_buffer.size() + sphere_normal_buffer.size()) * sizeof(float) + sphere_index_buffer.size() * sizeof(uint32_t);
    ptr = static_cast<unsigned char*>(device_.mapMemory(stage_buffer_.memory.memory, stage_buffer_.memory.offset + floor_buffer_size, sphere_buffer_size));
    std::memcpy(ptr, sphere_vertex_buffer.data(), sphere_vertex_buffer.size() * sizeof(float));
    std::memcpy(ptr + sphere_vertex_buffer.size() * sizeof(float), sphere_normal_buffer.data(), sphere_normal_buffer.size() * sizeof(float));
    std::memcpy(ptr + (sphere_vertex_buffer.size() + sphere_normal_buffer.size()) * sizeof(float), sphere_index_buffer.data(), sphere_index_buffer.size() * sizeof(uint32_t));
    device_.unmapMemory(stage_buffer_.memory.memory);

    buffer_create_info
      .setSize(sphere_buffer_size);
    sphere_buffer_.buffer = device_.createBuffer(buffer_create_info);
    sphere_buffer_.memory = AllocateDeviceMemory(sphere_buffer_.buffer);
    device_.bindBufferMemory(sphere_buffer_.buffer, sphere_buffer_.memory.memory, sphere_buffer_.memory.offset);

    transfer_commands[1].begin(begin_info);
    region
      .setSrcOffset(floor_buffer_size)
      .setDstOffset(0)
      .setSize(sphere_buffer_size);
    transfer_commands[1].copyBuffer(stage_buffer_.buffer, sphere_buffer_.buffer, region);
    transfer_commands[1].end();

    // Mesh loading
    const std::string mesh_filepath = "C:\\workspace\\twopi\\resources\\among_us_obj\\among us_scaled.obj";
    const auto mesh = geometry::MeshLoader{}.Load(mesh_filepath);

    const auto& mesh_vertices = mesh->Vertices();
    const auto& mesh_normals = mesh->Normals();
    const auto& mesh_tex_coords = mesh->TexCoords();
    const auto& mesh_indices = mesh->Indices();

    mesh_position_offset_ = 0;
    mesh_normal_offset_ = static_cast<uint32_t>(mesh_vertices.size()) * sizeof(float);
    mesh_tex_coord_offset_ = static_cast<uint32_t>(mesh_vertices.size() + mesh_normals.size()) * sizeof(float);
    mesh_index_offset_ = static_cast<uint32_t>(mesh_vertices.size() + mesh_normals.size() + mesh_tex_coords.size()) * sizeof(float);
    mesh_num_indices_ = static_cast<uint32_t>(mesh_indices.size());

    const auto mesh_buffer_size = mesh_index_offset_ + mesh_indices.size() * sizeof(uint32_t);
    ptr = static_cast<unsigned char*>(device_.mapMemory(stage_buffer_.memory.memory, stage_buffer_.memory.offset + floor_buffer_size + sphere_buffer_size, mesh_buffer_size));
    std::memcpy(ptr, mesh_vertices.data(), mesh_vertices.size() * sizeof(float));
    std::memcpy(ptr + mesh_normal_offset_, mesh_normals.data(), mesh_normals.size() * sizeof(float));
    std::memcpy(ptr + mesh_tex_coord_offset_, mesh_tex_coords.data(), mesh_tex_coords.size() * sizeof(float));
    std::memcpy(ptr + mesh_index_offset_, mesh_indices.data(), mesh_indices.size() * sizeof(uint32_t));
    device_.unmapMemory(stage_buffer_.memory.memory);

    buffer_create_info
      .setSize(mesh_buffer_size);
    mesh_buffer_.buffer = device_.createBuffer(buffer_create_info);
    mesh_buffer_.memory = AllocateDeviceMemory(mesh_buffer_.buffer);
    device_.bindBufferMemory(mesh_buffer_.buffer, mesh_buffer_.memory.memory, mesh_buffer_.memory.offset);

    transfer_commands[2].begin(begin_info);
    region
      .setSrcOffset(floor_buffer_size + sphere_buffer_size)
      .setDstOffset(0)
      .setSize(mesh_buffer_size);
    transfer_commands[2].copyBuffer(stage_buffer_.buffer, mesh_buffer_.buffer, region);
    transfer_commands[2].end();

    // Submit transfer commands
    vk::SubmitInfo submit_info;
    submit_info.setCommandBuffers(transfer_commands);
    graphics_queue_.submit(submit_info, transfer_fence_);

    const auto result = device_.waitForFences(transfer_fence_, true, UINT64_MAX);
  }

  void CleanupResources()
  {
    device_.unmapMemory(uniform_buffer_.memory.memory);
    device_.freeMemory(uniform_buffer_.memory.memory);
    device_.unmapMemory(dynamic_uniform_buffer_.memory.memory);
    device_.freeMemory(dynamic_uniform_buffer_.memory.memory);

    device_.destroyBuffer(floor_buffer_.buffer);
    device_.destroyBuffer(sphere_buffer_.buffer);
    device_.destroyBuffer(mesh_buffer_.buffer);
    device_.destroyBuffer(stage_buffer_.buffer);
    device_.destroyBuffer(uniform_buffer_.buffer);
    device_.destroyBuffer(dynamic_uniform_buffer_.buffer);
  }

  void CreateCommandBuffers()
  {
    vk::CommandBufferAllocateInfo allocate_info;
    allocate_info
      .setCommandPool(command_pool_)
      .setLevel(vk::CommandBufferLevel::ePrimary)
      .setCommandBufferCount(static_cast<uint32_t>(swapchain_images_.size()));
    draw_command_buffers_ = device_.allocateCommandBuffers(allocate_info);

    for (int i = 0; i < swapchain_images_.size(); i++)
    {
      auto& command_buffer = draw_command_buffers_[i];

      vk::CommandBufferBeginInfo begin_info;
      command_buffer.begin(begin_info);

      std::array<vk::ClearValue, 2> clear_values = {
        vk::ClearColorValue{ std::array<float, 4>{ 0.8f, 0.8f, 0.8f, 1.f } },
        vk::ClearDepthStencilValue{ 1.f, 0u }
      };

      vk::RenderPassBeginInfo render_pass_begin_info;
      render_pass_begin_info
        .setClearValues(clear_values)
        .setRenderPass(swapchain_render_pass_)
        .setFramebuffer(swapchain_framebuffers_[i])
        .setRenderArea(vk::Rect2D{ {0, 0}, {width_, height_} });
      command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

      // Sphere
      command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, color_pipeline_);

      command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout_, 0,
        { descriptor_sets_[i] }, { model_ubos_[i].stride, 0 });

      command_buffer.bindVertexBuffers(0,
        { sphere_buffer_.buffer, sphere_buffer_.buffer },
        { sphere_position_offset_, sphere_normal_offset_ });

      command_buffer.bindIndexBuffer(sphere_buffer_.buffer, sphere_index_offset_, vk::IndexType::eUint32);

      command_buffer.drawIndexed(sphere_num_indices_, 1, 0, 0, 0);

      // Mesh
      command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout_, 0,
        { descriptor_sets_[i] }, { model_ubos_[i].stride * 2, 0 });

      command_buffer.bindVertexBuffers(0,
        { mesh_buffer_.buffer, mesh_buffer_.buffer },
        { mesh_position_offset_, mesh_normal_offset_ });

      command_buffer.bindIndexBuffer(mesh_buffer_.buffer, mesh_index_offset_, vk::IndexType::eUint32);

      command_buffer.drawIndexed(mesh_num_indices_, 1, 0, 0, 0);
      
      // Floor
      command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, floor_pipeline_);

      command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout_, 0,
        { descriptor_sets_[i] }, { 0ull, 0ull });

      command_buffer.bindVertexBuffers(0,
        { floor_buffer_.buffer, floor_buffer_.buffer, floor_buffer_.buffer },
        { floor_position_offset_, floor_normal_offset_, floor_tex_coord_offset_ });

      command_buffer.bindIndexBuffer(floor_buffer_.buffer, floor_index_offset_, vk::IndexType::eUint32);

      command_buffer.drawIndexed(floor_num_indices_, 1, 0, 0, 0);

      command_buffer.endRenderPass();
      command_buffer.end();
    }
  }

  void DestroyCommandBuffers()
  {
    device_.freeCommandBuffers(command_pool_, draw_command_buffers_);
    draw_command_buffers_.clear();
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
    memory.offset = (device_memory_offset_ + requirements.alignment - 1ull) / requirements.alignment * requirements.alignment;
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

  Memory AllocatePersistentlyMappedMemory(vk::Image image)
  {
    return AllocatePersistentlyMappedMemory(device_.getImageMemoryRequirements(image));
  }

  Memory AllocatePersistentlyMappedMemory(vk::Buffer buffer)
  {
    return AllocatePersistentlyMappedMemory(device_.getBufferMemoryRequirements(buffer));
  }

  Memory AllocatePersistentlyMappedMemory(const vk::MemoryRequirements& requirements)
  {
    vk::MemoryAllocateInfo allocate_info;
    allocate_info
      .setMemoryTypeIndex(host_index_)
      .setAllocationSize(requirements.size);

    Memory memory;
    memory.memory = device_.allocateMemory(allocate_info);
    memory.offset = 0;
    memory.size = requirements.size;
    return memory;
  }

  vk::ShaderModule CreateShaderModule(const std::string& dirpath, const std::string& filename)
  {
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
    return device_.createShaderModule(shader_module_create_info);
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
  uint32_t device_index_ = 0;
  uint32_t host_index_ = 0;
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

  // Descriptor set
  vk::DescriptorSetLayout descriptor_set_layout_;
  vk::DescriptorPool descriptor_pool_;
  std::vector<vk::DescriptorSet> descriptor_sets_;

  // Pipelines
  vk::PipelineLayout pipeline_layout_;
  vk::Pipeline color_pipeline_;
  vk::Pipeline floor_pipeline_;

  // Commands
  vk::CommandPool command_pool_;
  vk::CommandPool transient_command_pool_;
  std::vector<vk::CommandBuffer> draw_command_buffers_;

  // Uniform buffers per swapchain framebuffer
  Buffer uniform_buffer_;
  unsigned char* uniform_buffer_map_;
  Buffer dynamic_uniform_buffer_;
  unsigned char* dynamic_uniform_buffer_map_;
  std::vector<UniformBuffer> camera_ubos_;
  std::vector<DynamicUniformBuffer> model_ubos_;
  std::vector<UniformBuffer> light_ubos_;
  std::vector<DynamicUniformBuffer> material_ubos_;
  uint32_t num_objects_ = 0;

  CameraUbo camera_;
  LightUbo lights_;
  MaterialUbo material_;
  ModelUbo floor_model_;
  ModelUbo mesh_model_;
  ModelUbo light_model_;

  // Stage buffer
  Buffer stage_buffer_;

  // Floor buffer
  Buffer floor_buffer_;
  uint32_t floor_position_offset_ = 0;
  uint32_t floor_normal_offset_ = 0;
  uint32_t floor_tex_coord_offset_ = 0;
  uint32_t floor_index_offset_ = 0;
  uint32_t floor_num_indices_ = 0;

  // Sphere buffer
  Buffer sphere_buffer_;
  uint32_t sphere_position_offset_ = 0;
  uint32_t sphere_normal_offset_ = 0;
  uint32_t sphere_index_offset_ = 0;
  uint32_t sphere_num_indices_ = 0;

  // Mesh
  Buffer mesh_buffer_;
  uint32_t mesh_position_offset_ = 0;
  uint32_t mesh_normal_offset_ = 0;
  uint32_t mesh_tex_coord_offset_ = 0;
  uint32_t mesh_index_offset_ = 0;
  uint32_t mesh_num_indices_ = 0;

  // Synchronization
  vk::Fence transfer_fence_;

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
}
}
