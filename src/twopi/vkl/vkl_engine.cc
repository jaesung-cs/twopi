#include <twopi/vkl/vkl_engine.h>

#include <iostream>
#include <optional>

#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

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
public:
  Impl() = delete;

  Impl(std::shared_ptr<window::Window> window)
  {
    glfw_window_handle_ = std::dynamic_pointer_cast<window::GlfwWindow>(window)->Handle();

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
    AllocateMemories();
  }

  void Cleanup()
  {
    FreeMemories();
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

  void AllocateMemories()
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

      if (properties & vk::MemoryPropertyFlagBits::eDeviceLocal)
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

  void FreeMemories()
  {
    device_.freeMemory(device_memory_);
    device_.freeMemory(host_memory_);
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
