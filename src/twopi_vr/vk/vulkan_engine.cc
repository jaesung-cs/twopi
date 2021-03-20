#include <twopi/vk/vulkan_engine.h>

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <chrono>

#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

namespace twopi
{
namespace vk
{
namespace
{
const std::vector<const char*> validation_layers = {
  "VK_LAYER_KHRONOS_validation",
};

const std::vector<const char*> device_extensions = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

bool CheckValidationLayerSupport()
{
  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  for (const char* layer_name : validation_layers)
  {
    bool layer_found = false;

    for (const auto& layer_properties : available_layers)
    {
      if (strcmp(layer_name, layer_properties.layerName) == 0)
      {
        layer_found = true;
        break;
      }
    }

    if (!layer_found)
      return false;
  }

  return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
  if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) ||
    (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT))
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

  return VK_FALSE;
}

void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& create_info)
{
  create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info.pfnUserCallback = DebugCallback;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr)
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  else
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr)
    func(instance, debugMessenger, pAllocator);
}

std::vector<char> ReadFile(const std::string& filename)
{
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open())
    throw std::runtime_error("Failed to open file!");

  size_t file_size = (size_t)file.tellg();
  std::vector<char> buffer(file_size);

  file.seekg(0);
  file.read(buffer.data(), file_size);

  file.close();

  return buffer;
}
}

VulkanEngine::VulkanEngine()
{
}

VulkanEngine::~VulkanEngine()
{
}

void VulkanEngine::Draw()
{
  vkWaitForFences(device_, 1, &in_flight_fences_[current_frame_], VK_TRUE, UINT64_MAX);

  uint32_t image_index;
  VkResult result = vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX, image_available_semaphores_[current_frame_], VK_NULL_HANDLE, &image_index);

  if (result == VK_ERROR_OUT_OF_DATE_KHR)
  {
    RecreateSwapchain();
    return;
  }
  else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    throw std::runtime_error("Failed to acquire swap chain image!");

  UpdateUniformBuffer(image_index);

  if (images_in_flight_[image_index] != VK_NULL_HANDLE)
    vkWaitForFences(device_, 1, &images_in_flight_[image_index], VK_TRUE, UINT64_MAX);
  images_in_flight_[image_index] = in_flight_fences_[current_frame_];

  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore wait_semaphores[] = { image_available_semaphores_[current_frame_] };
  VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = wait_semaphores;
  submit_info.pWaitDstStageMask = wait_stages;

  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffers_[image_index];

  VkSemaphore signal_semaphores[] = { render_finished_semaphores_[current_frame_] };
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = signal_semaphores;

  vkResetFences(device_, 1, &in_flight_fences_[current_frame_]);

  if (vkQueueSubmit(graphics_queue_, 1, &submit_info, in_flight_fences_[current_frame_]) != VK_SUCCESS)
    throw std::runtime_error("Failed to submit draw command buffer!");

  VkPresentInfoKHR present_info{};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = signal_semaphores;

  VkSwapchainKHR swapchains[] = { swapchain_ };
  present_info.swapchainCount = 1;
  present_info.pSwapchains = swapchains;

  present_info.pImageIndices = &image_index;

  result = vkQueuePresentKHR(present_queue_, &present_info);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebuffer_resized_)
  {
    framebuffer_resized_ = false;
    RecreateSwapchain();
  }
  else if (result != VK_SUCCESS)
    throw std::runtime_error("Failed to present swap chain image!");

  current_frame_ = (current_frame_ + 1) % max_frames_in_flight_;
}

void VulkanEngine::UpdateUniformBuffer(uint32_t current_image)
{
  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

  UniformBufferObject ubo{};
  ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.proj = glm::perspective(glm::radians(45.0f), swapchain_extent_.width / (float)swapchain_extent_.height, 0.1f, 10.0f);
  ubo.proj[1][1] *= -1;

  void* data;
  vkMapMemory(device_, uniform_buffers_memory_[current_image], 0, sizeof(ubo), 0, &data);
  memcpy(data, &ubo, sizeof(ubo));
  vkUnmapMemory(device_, uniform_buffers_memory_[current_image]);
}

void VulkanEngine::DeviceWaitIdle()
{
  vkDeviceWaitIdle(device_);
}

void VulkanEngine::Prepare(const PreparationOptions& preparation_params)
{
  hWindow_ = preparation_params.hWindow;
  enable_validation_layers_ = preparation_params.enable_validation_layers;

  CreateInstance(preparation_params.instance_extensions);

  if (enable_validation_layers_)
    SetupDebugMessenger();

  CreateSurface(preparation_params.hInstance, preparation_params.hWindow);

  PickPhysicalDevice(preparation_params.physical_device_selector);
  msaa_samples_ = GetMaxUsableSampleCount();

  CreateLogicalDevice();

  CreateDescriptorSetLayout();
  CreateCommandPool();

  CreateTextureImage("../resources/viking_room.png");
  CreateTextureImageView();
  CreateTextureSampler();

  LoadModel("../resources/viking_room.obj");
  CreateVertexBuffer();
  CreateIndexBuffer();

  CreateSwapchain();
  CreateSwapchainImageViews();

  CreateSyncObjects();

  CreateRenderPass();

  CreateUniformBuffers();

  CreateDescriptorPool();
  CreateDescriptorSets();

  CreateGraphicsPipeline();

  CreateColorResources();
  CreateDepthResources();

  CreateFramebuffers();

  CreateCommandBuffers();
}

void VulkanEngine::RecreateSwapchain()
{
  /*
  int width = 0, height = 0;
  glfwGetFramebufferSize(window, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(window, &width, &height);
    glfwWaitEvents();
  }
  */

  RECT area;
  GetClientRect(hWindow_, &area);
  int width = area.right;
  int height = area.bottom;

  DeviceWaitIdle();

  CleanupSwapchain();

  CreateSwapchain();
  CreateSwapchainImageViews();

  CreateRenderPass();

  CreateDescriptorPool();
  CreateDescriptorSets();

  CreateGraphicsPipeline();

  CreateColorResources();
  CreateDepthResources();

  CreateFramebuffers();

  CreateUniformBuffers();

  CreateCommandBuffers();
}

void VulkanEngine::CleanupSwapchain()
{
  vkDestroyImageView(device_, depth_image_view_, nullptr);
  vkDestroyImage(device_, depth_image_, nullptr);
  vkFreeMemory(device_, depth_image_memory_, nullptr);

  vkDestroyImageView(device_, color_image_view_, nullptr);
  vkDestroyImage(device_, color_image_, nullptr);
  vkFreeMemory(device_, color_image_memory_, nullptr);

  for (auto framebuffer : swapchain_framebuffers_)
    vkDestroyFramebuffer(device_, framebuffer, nullptr);

  vkFreeCommandBuffers(device_, command_pool_, static_cast<uint32_t>(command_buffers_.size()), command_buffers_.data());

  vkDestroyPipeline(device_, graphics_pipeline_, nullptr);
  vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
  vkDestroyRenderPass(device_, render_pass_, nullptr);

  for (auto image_view : swapchain_image_views_)
    vkDestroyImageView(device_, image_view, nullptr);

  vkDestroySwapchainKHR(device_, swapchain_, nullptr);

  for (size_t i = 0; i < swapchain_images_.size(); i++)
  {
    vkDestroyBuffer(device_, uniform_buffers_[i], nullptr);
    vkFreeMemory(device_, uniform_buffers_memory_[i], nullptr);
  }

  vkDestroyDescriptorPool(device_, descriptor_pool_, nullptr);
}

void VulkanEngine::Cleanup()
{
  CleanupSwapchain();

  vkDestroySampler(device_, texture_sampler_, nullptr);
  vkDestroyImageView(device_, texture_image_view_, nullptr);

  vkDestroyImage(device_, texture_image_, nullptr);
  vkFreeMemory(device_, texture_image_memory_, nullptr);

  vkDestroyDescriptorSetLayout(device_, descriptor_set_layout_, nullptr);

  vkDestroyBuffer(device_, index_buffer_, nullptr);
  vkFreeMemory(device_, index_buffer_memory_, nullptr);

  vkDestroyBuffer(device_, vertex_buffer_, nullptr);
  vkFreeMemory(device_, vertex_buffer_memory_, nullptr);

  for (size_t i = 0; i < max_frames_in_flight_; i++)
  {
    vkDestroySemaphore(device_, render_finished_semaphores_[i], nullptr);
    vkDestroySemaphore(device_, image_available_semaphores_[i], nullptr);
    vkDestroyFence(device_, in_flight_fences_[i], nullptr);
  }

  vkDestroyCommandPool(device_, command_pool_, nullptr);

  vkDestroyDevice(device_, nullptr);

  if (enable_validation_layers_)
    DestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);

  vkDestroySurfaceKHR(instance_, surface_, nullptr);
  vkDestroyInstance(instance_, nullptr);
}

void VulkanEngine::CreateInstance(std::vector<std::string> extensions)
{
  if (enable_validation_layers_ && !CheckValidationLayerSupport())
    throw std::runtime_error("Validation layers requested, but not available!");

  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "TwoPi Vk";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "TwoPi Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;

  // Add required extensions by vulkan engine
  std::unordered_set<std::string> extensions_set(extensions.begin(), extensions.end());
  for (const auto& required_extension : GetRequiredExtensions())
  {
    if (extensions_set.find(required_extension) == extensions_set.end())
    {
      extensions_set.insert(required_extension);
      extensions.emplace_back(required_extension);
    }
  }

  std::vector<const char*> extensions_cstr;
  for (const auto& extension : extensions)
    extensions_cstr.push_back(extension.c_str());

  create_info.enabledExtensionCount = static_cast<uint32_t>(extensions_cstr.size());
  create_info.ppEnabledExtensionNames = extensions_cstr.data();

  VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
  if (enable_validation_layers_)
  {
    create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
    create_info.ppEnabledLayerNames = validation_layers.data();

    PopulateDebugMessengerCreateInfo(debug_create_info);
    create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
  }
  else
  {
    create_info.enabledLayerCount = 0;
    create_info.pNext = nullptr;
  }

  if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS)
    throw std::runtime_error("Failed to create instance!");
}

std::vector<const char*> VulkanEngine::GetRequiredExtensions()
{
  std::vector<const char*> extensions;

  extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
  extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

  if (enable_validation_layers_)
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  return extensions;
}

void VulkanEngine::SetupDebugMessenger()
{
  VkDebugUtilsMessengerCreateInfoEXT create_info;
  PopulateDebugMessengerCreateInfo(create_info);

  if (CreateDebugUtilsMessengerEXT(instance_, &create_info, nullptr, &debug_messenger_) != VK_SUCCESS)
    throw std::runtime_error("Failed to set up debug messenger!");
}

void VulkanEngine::CreateSurface(const HINSTANCE hInstance, const HWND hWindow)
{
  VkWin32SurfaceCreateInfoKHR create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  create_info.flags = 0;
  create_info.hinstance = hInstance;
  create_info.hwnd = hWindow;

  if (vkCreateWin32SurfaceKHR(instance_, &create_info, nullptr, &surface_) != VK_SUCCESS)
    throw std::runtime_error("Failed to set up surface!");
}

void VulkanEngine::PickPhysicalDevice(std::function<VkPhysicalDevice(VkInstance)> physical_device_selector)
{
  if (physical_device_selector)
    physical_device_ = physical_device_selector(instance_);
  else
    PickBestPhysicalDevice();
}

void VulkanEngine::PickBestPhysicalDevice()
{
  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);

  if (device_count == 0)
    throw std::runtime_error("Failed to find GPUs with Vulkan support!");

  std::vector<VkPhysicalDevice> devices(device_count);
  vkEnumeratePhysicalDevices(instance_, &device_count, devices.data());

  for (const auto& device : devices)
  {
    if (IsDeviceSuitable(device)) {
      physical_device_ = device;
      break;
    }
  }

  if (physical_device_ == VK_NULL_HANDLE)
    throw std::runtime_error("Failed to find a suitable GPU!");
}

bool VulkanEngine::IsDeviceSuitable(VkPhysicalDevice device)
{
  QueueFamilyIndices indices = FindQueueFamilies(device);

  bool extensions_supported = CheckDeviceExtensionSupport(device);
  bool swapchain_adequate = false;
  if (extensions_supported)
  {
    SwapchainSupportDetails swapchain_support = QuerySwapchainSupport(device);
    swapchain_adequate = !swapchain_support.formats.empty() && !swapchain_support.present_modes.empty();
  }

  VkPhysicalDeviceFeatures supported_features;
  vkGetPhysicalDeviceFeatures(device, &supported_features);

  return indices.IsComplete() && extensions_supported && swapchain_adequate && supported_features.samplerAnisotropy;
}

VulkanEngine::QueueFamilyIndices VulkanEngine::FindQueueFamilies(VkPhysicalDevice device)
{
  QueueFamilyIndices indices;

  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

  int i = 0;
  for (const auto& queue_family : queue_families)
  {
    if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
      indices.graphics_family = i;

    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &present_support);

    if (present_support)
      indices.present_family = i;

    if (indices.IsComplete())
      break;

    i++;
  }

  return indices;
}

bool VulkanEngine::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
  uint32_t extension_count;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
  std::vector<VkExtensionProperties> available_extensions(extension_count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

  std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

  for (const auto& extension : available_extensions)
    required_extensions.erase(extension.extensionName);

  return required_extensions.empty();
}

VulkanEngine::SwapchainSupportDetails VulkanEngine::QuerySwapchainSupport(VkPhysicalDevice device)
{
  SwapchainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);
  if (formatCount != 0)
  {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
  }

  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &present_mode_count, nullptr);
  if (present_mode_count != 0)
  {
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &present_mode_count, details.present_modes.data());
  }

  return details;
}

VkSampleCountFlagBits VulkanEngine::GetMaxUsableSampleCount()
{
  VkPhysicalDeviceProperties physical_device_properties;
  vkGetPhysicalDeviceProperties(physical_device_, &physical_device_properties);

  VkSampleCountFlags counts = physical_device_properties.limits.framebufferColorSampleCounts & physical_device_properties.limits.framebufferDepthSampleCounts;
  if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
  if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
  if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
  if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
  if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
  if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;

  return VK_SAMPLE_COUNT_1_BIT;
}

void VulkanEngine::CreateLogicalDevice()
{
  QueueFamilyIndices indices = FindQueueFamilies(physical_device_);

  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  std::set<uint32_t> unique_queue_families = { indices.graphics_family.value(), indices.present_family.value() };

  float queue_priority = 1.0f;
  for (uint32_t queue_family : unique_queue_families)
  {
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_family;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    queue_create_infos.push_back(queue_create_info);
  }

  VkPhysicalDeviceFeatures device_features{};
  device_features.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
  create_info.pQueueCreateInfos = queue_create_infos.data();

  create_info.pEnabledFeatures = &device_features;

  create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
  create_info.ppEnabledExtensionNames = device_extensions.data();

  if (enable_validation_layers_)
  {
    create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
    create_info.ppEnabledLayerNames = validation_layers.data();
  }
  else
    create_info.enabledLayerCount = 0;

  if (vkCreateDevice(physical_device_, &create_info, nullptr, &device_) != VK_SUCCESS)
    throw std::runtime_error("failed to create logical device!");

  vkGetDeviceQueue(device_, indices.graphics_family.value(), 0, &graphics_queue_);
  vkGetDeviceQueue(device_, indices.present_family.value(), 0, &present_queue_);
}

void VulkanEngine::CreateSwapchain()
{
  SwapchainSupportDetails swapchain_support = QuerySwapchainSupport(physical_device_);

  VkSurfaceFormatKHR surface_format = ChooseSwapchainSurfaceFormat(swapchain_support.formats);
  VkPresentModeKHR present_mode = ChooseSwapchainPresentMode(swapchain_support.present_modes);
  VkExtent2D extent = ChooseSwapchainExtent(swapchain_support.capabilities);

  uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
  if (swapchain_support.capabilities.maxImageCount > 0 && image_count > swapchain_support.capabilities.maxImageCount)
    image_count = swapchain_support.capabilities.maxImageCount;

  VkSwapchainCreateInfoKHR create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = surface_;

  create_info.minImageCount = image_count;
  create_info.imageFormat = surface_format.format;
  create_info.imageColorSpace = surface_format.colorSpace;
  create_info.imageExtent = extent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = FindQueueFamilies(physical_device_);
  uint32_t queue_family_indices[] = { indices.graphics_family.value(), indices.present_family.value() };

  if (indices.graphics_family != indices.present_family)
  {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = queue_family_indices;
  }
  else
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

  create_info.preTransform = swapchain_support.capabilities.currentTransform;
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  create_info.presentMode = present_mode;
  create_info.clipped = VK_TRUE;

  if (vkCreateSwapchainKHR(device_, &create_info, nullptr, &swapchain_) != VK_SUCCESS)
    throw std::runtime_error("Failed to create swapchain!");

  vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, nullptr);
  swapchain_images_.resize(image_count);
  vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, swapchain_images_.data());

  swapchain_image_format_ = surface_format.format;
  swapchain_extent_ = extent;
}

VkSurfaceFormatKHR VulkanEngine::ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats)
{
  for (const auto& available_format : available_formats)
  {
    if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      return available_format;
  }

  return available_formats[0];
}

VkPresentModeKHR VulkanEngine::ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes)
{
  for (const auto& available_present_mode : available_present_modes)
  {
    if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
      return available_present_mode;
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanEngine::ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
  if (capabilities.currentExtent.width != UINT32_MAX)
    return capabilities.currentExtent;

  else
  {
    RECT area;
    GetClientRect(hWindow_, &area);

    int width = area.right;
    int height = area.bottom;

    VkExtent2D actual_extent = {
      static_cast<uint32_t>(width),
      static_cast<uint32_t>(height),
    };

    actual_extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actual_extent.width));
    actual_extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actual_extent.height));

    return actual_extent;
  }
}

void VulkanEngine::CreateSwapchainImageViews()
{
  swapchain_image_views_.resize(swapchain_images_.size());

  for (uint32_t i = 0; i < swapchain_images_.size(); i++)
    swapchain_image_views_[i] = CreateImageView(swapchain_images_[i], swapchain_image_format_, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

VkImageView VulkanEngine::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels)
{
  VkImageViewCreateInfo view_info{};
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image = image;
  view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  view_info.format = format;
  view_info.subresourceRange.aspectMask = aspect_flags;
  view_info.subresourceRange.baseMipLevel = 0;
  view_info.subresourceRange.levelCount = mip_levels;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount = 1;

  VkImageView image_view_;
  if (vkCreateImageView(device_, &view_info, nullptr, &image_view_) != VK_SUCCESS)
    throw std::runtime_error("Failed to create texture image view!");

  return image_view_;
}

void VulkanEngine::CreateRenderPass()
{
  VkAttachmentDescription color_attachment{};
  color_attachment.format = swapchain_image_format_;
  color_attachment.samples = msaa_samples_;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription depth_attachment{};
  depth_attachment.format = FindDepthFormat();
  depth_attachment.samples = msaa_samples_;
  depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription color_attachment_resolve{};
  color_attachment_resolve.format = swapchain_image_format_;
  color_attachment_resolve.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment_resolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment_resolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment_resolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment_resolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment_resolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_ref{};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depth_attachment_ref{};
  depth_attachment_ref.attachment = 1;
  depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference color_attachment_resolve_ref{};
  color_attachment_resolve_ref.attachment = 2;
  color_attachment_resolve_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;
  subpass.pDepthStencilAttachment = &depth_attachment_ref;
  subpass.pResolveAttachments = &color_attachment_resolve_ref;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  std::array<VkAttachmentDescription, 3> attachments = { color_attachment, depth_attachment, color_attachment_resolve };
  VkRenderPassCreateInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
  render_pass_info.pAttachments = attachments.data();
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;
  render_pass_info.dependencyCount = 1;
  render_pass_info.pDependencies = &dependency;

  if (vkCreateRenderPass(device_, &render_pass_info, nullptr, &render_pass_) != VK_SUCCESS)
    throw std::runtime_error("Failed to create render pass!");
}

VkFormat VulkanEngine::FindDepthFormat()
{
  return FindSupportedFormat(
    { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
    VK_IMAGE_TILING_OPTIMAL,
    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
  );
}

VkFormat VulkanEngine::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
  for (VkFormat format : candidates)
  {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physical_device_, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
      return format;
    
    else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
      return format;
  }

  throw std::runtime_error("Failed to find supported format!");
}

void VulkanEngine::CreateDescriptorSetLayout()
{
  VkDescriptorSetLayoutBinding ubo_layout_binding{};
  ubo_layout_binding.binding = 0;
  ubo_layout_binding.descriptorCount = 1;
  ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  ubo_layout_binding.pImmutableSamplers = nullptr;
  ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutBinding sampler_layout_binding{};
  sampler_layout_binding.binding = 1;
  sampler_layout_binding.descriptorCount = 1;
  sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  sampler_layout_binding.pImmutableSamplers = nullptr;
  sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array<VkDescriptorSetLayoutBinding, 2> bindings = { ubo_layout_binding, sampler_layout_binding };
  VkDescriptorSetLayoutCreateInfo layout_info{};
  layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
  layout_info.pBindings = bindings.data();

  if (vkCreateDescriptorSetLayout(device_, &layout_info, nullptr, &descriptor_set_layout_) != VK_SUCCESS)
    throw std::runtime_error("Failed to create descriptor set layout!");
}

void VulkanEngine::CreateGraphicsPipeline()
{
  auto vert_shader_code = ReadFile("shaders/mesh.vert.spv");
  auto frag_shader_code = ReadFile("shaders/mesh.frag.spv");

  VkShaderModule vert_shader_module = CreateShaderModule(vert_shader_code);
  VkShaderModule frag_shader_module = CreateShaderModule(frag_shader_code);

  VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
  vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vert_shader_stage_info.module = vert_shader_module;
  vert_shader_stage_info.pName = "main";

  VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
  frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  frag_shader_stage_info.module = frag_shader_module;
  frag_shader_stage_info.pName = "main";

  VkPipelineShaderStageCreateInfo shader_stages[] = { vert_shader_stage_info, frag_shader_stage_info };

  VkPipelineVertexInputStateCreateInfo vertex_input_info{};
  vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  auto binding_description = Vertex::GetBindingDescription();
  auto attribute_descriptions = Vertex::GetAttributeDescriptions();

  vertex_input_info.vertexBindingDescriptionCount = 1;
  vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
  vertex_input_info.pVertexBindingDescriptions = &binding_description;
  vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

  VkPipelineInputAssemblyStateCreateInfo input_assembly{};
  input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)swapchain_extent_.width;
  viewport.height = (float)swapchain_extent_.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = { 0, 0 };
  scissor.extent = swapchain_extent_;

  VkPipelineViewportStateCreateInfo viewport_state{};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.viewportCount = 1;
  viewport_state.pViewports = &viewport;
  viewport_state.scissorCount = 1;
  viewport_state.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = msaa_samples_;

  VkPipelineDepthStencilStateCreateInfo depth_stencil{};
  depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depth_stencil.depthTestEnable = VK_TRUE;
  depth_stencil.depthWriteEnable = VK_TRUE;
  depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
  depth_stencil.depthBoundsTestEnable = VK_FALSE;
  depth_stencil.stencilTestEnable = VK_FALSE;

  VkPipelineColorBlendAttachmentState color_blend_attachment{};
  color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  color_blend_attachment.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo color_blending{};
  color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blending.logicOpEnable = VK_FALSE;
  color_blending.logicOp = VK_LOGIC_OP_COPY;
  color_blending.attachmentCount = 1;
  color_blending.pAttachments = &color_blend_attachment;
  color_blending.blendConstants[0] = 0.0f;
  color_blending.blendConstants[1] = 0.0f;
  color_blending.blendConstants[2] = 0.0f;
  color_blending.blendConstants[3] = 0.0f;

  VkPipelineLayoutCreateInfo pipeline_layout_info{};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = 1;
  pipeline_layout_info.pSetLayouts = &descriptor_set_layout_;

  if (vkCreatePipelineLayout(device_, &pipeline_layout_info, nullptr, &pipeline_layout_) != VK_SUCCESS)
    throw std::runtime_error("Failed to create pipeline layout!");

  VkGraphicsPipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = 2;
  pipeline_info.pStages = shader_stages;
  pipeline_info.pVertexInputState = &vertex_input_info;
  pipeline_info.pInputAssemblyState = &input_assembly;
  pipeline_info.pViewportState = &viewport_state;
  pipeline_info.pRasterizationState = &rasterizer;
  pipeline_info.pMultisampleState = &multisampling;
  pipeline_info.pDepthStencilState = &depth_stencil;
  pipeline_info.pColorBlendState = &color_blending;
  pipeline_info.layout = pipeline_layout_;
  pipeline_info.renderPass = render_pass_;
  pipeline_info.subpass = 0;
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

  if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline_) != VK_SUCCESS)
    throw std::runtime_error("Failed to create graphics pipeline!");

  vkDestroyShaderModule(device_, frag_shader_module, nullptr);
  vkDestroyShaderModule(device_, vert_shader_module, nullptr);
}

VkShaderModule VulkanEngine::CreateShaderModule(const std::vector<char>& code)
{
  VkShaderModuleCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = code.size();
  create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

  VkShaderModule shader_module;
  if (vkCreateShaderModule(device_, &create_info, nullptr, &shader_module) != VK_SUCCESS)
    throw std::runtime_error("Failed to create shader module!");

  return shader_module;
}

void VulkanEngine::CreateCommandPool()
{
  QueueFamilyIndices queue_family_indices = FindQueueFamilies(physical_device_);

  VkCommandPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value();

  if (vkCreateCommandPool(device_, &pool_info, nullptr, &command_pool_) != VK_SUCCESS)
    throw std::runtime_error("Failed to create graphics command pool!");
}

void VulkanEngine::CreateColorResources()
{
  VkFormat color_format = swapchain_image_format_;

  CreateImage(swapchain_extent_.width, swapchain_extent_.height, 1, msaa_samples_, color_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, color_image_, color_image_memory_);
  color_image_view_ = CreateImageView(color_image_, color_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void VulkanEngine::CreateDepthResources()
{
  VkFormat depth_format = FindDepthFormat();

  CreateImage(swapchain_extent_.width, swapchain_extent_.height, 1, msaa_samples_, depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth_image_, depth_image_memory_);
  depth_image_view_ = CreateImageView(depth_image_, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void VulkanEngine::CreateImage(uint32_t width, uint32_t height, uint32_t mip_levels, VkSampleCountFlagBits num_samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& image_memory)
{
  VkImageCreateInfo image_info{};
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.imageType = VK_IMAGE_TYPE_2D;
  image_info.extent.width = width;
  image_info.extent.height = height;
  image_info.extent.depth = 1;
  image_info.mipLevels = mip_levels;
  image_info.arrayLayers = 1;
  image_info.format = format;
  image_info.tiling = tiling;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.usage = usage;
  image_info.samples = num_samples;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateImage(device_, &image_info, nullptr, &image) != VK_SUCCESS)
    throw std::runtime_error("Failed to create image!");

  VkMemoryRequirements mem_requirements;
  vkGetImageMemoryRequirements(device_, image, &mem_requirements);

  VkMemoryAllocateInfo alloc_info{};

  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = mem_requirements.size;
  alloc_info.memoryTypeIndex = FindMemoryType(mem_requirements.memoryTypeBits, properties);

  if (vkAllocateMemory(device_, &alloc_info, nullptr, &image_memory) != VK_SUCCESS)
    throw std::runtime_error("Failed to allocate image memory!");

  vkBindImageMemory(device_, image, image_memory, 0);
}

uint32_t VulkanEngine::FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties)
{
  VkPhysicalDeviceMemoryProperties mem_properties;
  vkGetPhysicalDeviceMemoryProperties(physical_device_, &mem_properties);

  for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
  {
    if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
      return i;
  }

  throw std::runtime_error("Failed to find suitable memory type!");
}

void VulkanEngine::CreateFramebuffers()
{
  swapchain_framebuffers_.resize(swapchain_image_views_.size(), nullptr);

  for (size_t i = 0; i < swapchain_image_views_.size(); i++)
  {
    std::array<VkImageView, 3> attachments = {
        color_image_view_,
        depth_image_view_,
        swapchain_image_views_[i]
    };

    VkFramebufferCreateInfo framebuffer_info{};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = render_pass_;
    framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebuffer_info.pAttachments = attachments.data();
    framebuffer_info.width = swapchain_extent_.width;
    framebuffer_info.height = swapchain_extent_.height;
    framebuffer_info.layers = 1;

    if (vkCreateFramebuffer(device_, &framebuffer_info, nullptr, &swapchain_framebuffers_[i]) != VK_SUCCESS)
      throw std::runtime_error("Failed to create framebuffer!");
  }
}

void VulkanEngine::CreateTextureImage(const std::string& texture_path)
{
  int tex_width, tex_height, tex_channels;
  stbi_uc* pixels = stbi_load(texture_path.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
  VkDeviceSize image_size = tex_width * tex_height * 4;
  mip_levels_ = static_cast<uint32_t>(std::floor(std::log2(std::max(tex_width, tex_height)))) + 1;

  if (!pixels)
    throw std::runtime_error("Failed to load texture image!");

  VkBuffer staging_buffer;
  VkDeviceMemory staging_buffer_memory;
  CreateBuffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

  void* data;
  vkMapMemory(device_, staging_buffer_memory, 0, image_size, 0, &data);
  memcpy(data, pixels, static_cast<size_t>(image_size));
  vkUnmapMemory(device_, staging_buffer_memory);

  stbi_image_free(pixels);

  CreateImage(tex_width, tex_height, mip_levels_, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture_image_, texture_image_memory_);

  TransitionImageLayout(texture_image_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_levels_);
  CopyBufferToImage(staging_buffer, texture_image_, static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height));
  //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

  vkDestroyBuffer(device_, staging_buffer, nullptr);
  vkFreeMemory(device_, staging_buffer_memory, nullptr);

  GenerateMipmaps(texture_image_, VK_FORMAT_R8G8B8A8_SRGB, tex_width, tex_height, mip_levels_);
}

void VulkanEngine::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory)
{
  VkBufferCreateInfo buffer_info{};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device_, &buffer_info, nullptr, &buffer) != VK_SUCCESS)
    throw std::runtime_error("Failed to create buffer!");

  VkMemoryRequirements mem_requirements;
  vkGetBufferMemoryRequirements(device_, buffer, &mem_requirements);

  VkMemoryAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = mem_requirements.size;
  alloc_info.memoryTypeIndex = FindMemoryType(mem_requirements.memoryTypeBits, properties);

  if (vkAllocateMemory(device_, &alloc_info, nullptr, &buffer_memory) != VK_SUCCESS)
    throw std::runtime_error("Failed to allocate buffer memory!");

  vkBindBufferMemory(device_, buffer, buffer_memory, 0);
}

void VulkanEngine::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels)
{
  VkCommandBuffer command_buffer = BeginSingleTimeCommands();

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = mip_levels;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  VkPipelineStageFlags source_stage;
  VkPipelineStageFlags destination_stage;

  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
  {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else
    throw std::invalid_argument("unsupported layout transition!");

  vkCmdPipelineBarrier(
    command_buffer,
    source_stage, destination_stage,
    0,
    0, nullptr,
    0, nullptr,
    1, &barrier
  );

  EndSingleTimeCommands(command_buffer);
}

VkCommandBuffer VulkanEngine::BeginSingleTimeCommands()
{
  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandPool = command_pool_;
  alloc_info.commandBufferCount = 1;

  VkCommandBuffer command_buffer;
  vkAllocateCommandBuffers(device_, &alloc_info, &command_buffer);

  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(command_buffer, &begin_info);

  return command_buffer;
}

void VulkanEngine::EndSingleTimeCommands(VkCommandBuffer command_buffer)
{
  vkEndCommandBuffer(command_buffer);

  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer;

  vkQueueSubmit(graphics_queue_, 1, &submit_info, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphics_queue_);

  vkFreeCommandBuffers(device_, command_pool_, 1, &command_buffer);
}

void VulkanEngine::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
  VkCommandBuffer command_buffer = BeginSingleTimeCommands();

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = { 0, 0, 0 };
  region.imageExtent = {
    width,
    height,
    1
  };

  vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  EndSingleTimeCommands(command_buffer);
}

void VulkanEngine::GenerateMipmaps(VkImage image, VkFormat image_format, int32_t tex_width, int32_t tex_height, uint32_t mip_levels)
{
  // Check if image format supports linear blitting
  VkFormatProperties format_properties;
  vkGetPhysicalDeviceFormatProperties(physical_device_, image_format, &format_properties);

  if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    throw std::runtime_error("Texture image format does not support linear blitting!");

  VkCommandBuffer command_buffer = BeginSingleTimeCommands();

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image = image;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = 1;

  int32_t mip_width = tex_width;
  int32_t mip_height = tex_height;

  for (uint32_t i = 1; i < mip_levels; i++)
  {
    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(command_buffer,
      VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
      0, nullptr,
      0, nullptr,
      1, &barrier
    );

    VkImageBlit blit{};
    blit.srcOffsets[0] = { 0, 0, 0 };
    blit.srcOffsets[1] = { mip_width, mip_height, 1 };
    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.srcSubresource.mipLevel = i - 1;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = 1;
    blit.dstOffsets[0] = { 0, 0, 0 };
    blit.dstOffsets[1] = { mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1 };
    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.dstSubresource.mipLevel = i;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount = 1;

    vkCmdBlitImage(command_buffer,
      image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1, &blit,
      VK_FILTER_LINEAR);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(command_buffer,
      VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
      0, nullptr,
      0, nullptr,
      1, &barrier);

    if (mip_width > 1) mip_width /= 2;
    if (mip_height > 1) mip_height /= 2;
  }

  barrier.subresourceRange.baseMipLevel = mip_levels - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(command_buffer,
    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
    0, nullptr,
    0, nullptr,
    1, &barrier
  );

  EndSingleTimeCommands(command_buffer);
}

void VulkanEngine::CreateTextureImageView()
{
  texture_image_view_ = CreateImageView(texture_image_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mip_levels_);
}

void VulkanEngine::CreateTextureSampler()
{
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(physical_device_, &properties);

  VkSamplerCreateInfo sampler_info{};
  sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_info.magFilter = VK_FILTER_LINEAR;
  sampler_info.minFilter = VK_FILTER_LINEAR;
  sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.anisotropyEnable = VK_TRUE;
  sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
  sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  sampler_info.unnormalizedCoordinates = VK_FALSE;
  sampler_info.compareEnable = VK_FALSE;
  sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
  sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  sampler_info.minLod = 0.0f;
  sampler_info.maxLod = static_cast<float>(mip_levels_);
  sampler_info.mipLodBias = 0.0f;

  if (vkCreateSampler(device_, &sampler_info, nullptr, &texture_sampler_) != VK_SUCCESS)
    throw std::runtime_error("Failed to create texture sampler!");
}

void VulkanEngine::LoadModel(const std::string& obj_filename)
{
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, obj_filename.c_str()))
    throw std::runtime_error(warn + err);

  std::unordered_map<Vertex, uint32_t, Vertex::Hash> unique_vertices{};

  for (const auto& shape : shapes)
  {
    for (const auto& index : shape.mesh.indices)
    {
      Vertex vertex{};

      vertex.pos = {
        attrib.vertices[3 * index.vertex_index + 0],
        attrib.vertices[3 * index.vertex_index + 1],
        attrib.vertices[3 * index.vertex_index + 2]
      };

      vertex.texCoord = {
        attrib.texcoords[2 * index.texcoord_index + 0],
        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
      };

      vertex.color = { 1.0f, 1.0f, 1.0f };

      if (unique_vertices.count(vertex) == 0)
      {
        unique_vertices[vertex] = static_cast<uint32_t>(vertices_.size());
        vertices_.push_back(vertex);
      }

      indices_.push_back(unique_vertices[vertex]);
    }
  }
}

void VulkanEngine::CreateVertexBuffer()
{
  VkDeviceSize buffer_size = sizeof(vertices_[0]) * vertices_.size();

  VkBuffer staging_buffer;
  VkDeviceMemory staging_buffer_memory;
  CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

  void* data;
  vkMapMemory(device_, staging_buffer_memory, 0, buffer_size, 0, &data);
  memcpy(data, vertices_.data(), (size_t)buffer_size);
  vkUnmapMemory(device_, staging_buffer_memory);

  CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer_, vertex_buffer_memory_);

  CopyBuffer(staging_buffer, vertex_buffer_, buffer_size);

  vkDestroyBuffer(device_, staging_buffer, nullptr);
  vkFreeMemory(device_, staging_buffer_memory, nullptr);
}

void VulkanEngine::CreateIndexBuffer()
{
  VkDeviceSize buffer_size = sizeof(indices_[0]) * indices_.size();

  VkBuffer staging_buffer;
  VkDeviceMemory staging_buffer_memory;
  CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

  void* data;
  vkMapMemory(device_, staging_buffer_memory, 0, buffer_size, 0, &data);
  memcpy(data, indices_.data(), (size_t)buffer_size);
  vkUnmapMemory(device_, staging_buffer_memory);

  CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index_buffer_, index_buffer_memory_);

  CopyBuffer(staging_buffer, index_buffer_, buffer_size);

  vkDestroyBuffer(device_, staging_buffer, nullptr);
  vkFreeMemory(device_, staging_buffer_memory, nullptr);
}

void VulkanEngine::CopyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size)
{
  VkCommandBuffer command_buffer = BeginSingleTimeCommands();

  VkBufferCopy copy_region{};
  copy_region.size = size;
  vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

  EndSingleTimeCommands(command_buffer);
}

void VulkanEngine::CreateUniformBuffers()
{
  VkDeviceSize buffer_size = sizeof(UniformBufferObject);

  uniform_buffers_.resize(swapchain_images_.size());
  uniform_buffers_memory_.resize(swapchain_images_.size());

  for (size_t i = 0; i < swapchain_images_.size(); i++)
    CreateBuffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniform_buffers_[i], uniform_buffers_memory_[i]);
}

void VulkanEngine::CreateDescriptorPool()
{
  std::array<VkDescriptorPoolSize, 2> pool_sizes{};
  pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  pool_sizes[0].descriptorCount = static_cast<uint32_t>(swapchain_images_.size());
  pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  pool_sizes[1].descriptorCount = static_cast<uint32_t>(swapchain_images_.size());

  VkDescriptorPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
  pool_info.pPoolSizes = pool_sizes.data();
  pool_info.maxSets = static_cast<uint32_t>(swapchain_images_.size());

  if (vkCreateDescriptorPool(device_, &pool_info, nullptr, &descriptor_pool_) != VK_SUCCESS)
    throw std::runtime_error("Failed to create descriptor pool!");
}

void VulkanEngine::CreateDescriptorSets()
{
  std::vector<VkDescriptorSetLayout> layouts(swapchain_images_.size(), descriptor_set_layout_);

  VkDescriptorSetAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = descriptor_pool_;
  alloc_info.descriptorSetCount = static_cast<uint32_t>(swapchain_images_.size());
  alloc_info.pSetLayouts = layouts.data();

  descriptor_sets_.resize(swapchain_images_.size());
  if (vkAllocateDescriptorSets(device_, &alloc_info, descriptor_sets_.data()) != VK_SUCCESS)
    throw std::runtime_error("failed to allocate descriptor sets!");

  for (size_t i = 0; i < swapchain_images_.size(); i++)
  {
    VkDescriptorBufferInfo buffer_info{};
    buffer_info.buffer = uniform_buffers_[i];
    buffer_info.offset = 0;
    buffer_info.range = sizeof(UniformBufferObject);

    VkDescriptorImageInfo image_info{};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView = texture_image_view_;
    image_info.sampler = texture_sampler_;

    std::array<VkWriteDescriptorSet, 2> descriptor_writes{};

    descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[0].dstSet = descriptor_sets_[i];
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].dstArrayElement = 0;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].pBufferInfo = &buffer_info;

    descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[1].dstSet = descriptor_sets_[i];
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].dstArrayElement = 0;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].pImageInfo = &image_info;

    vkUpdateDescriptorSets(device_, static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
  }
}

void VulkanEngine::CreateCommandBuffers()
{
  command_buffers_.resize(swapchain_framebuffers_.size());

  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = command_pool_;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = (uint32_t)command_buffers_.size();

  if (vkAllocateCommandBuffers(device_, &alloc_info, command_buffers_.data()) != VK_SUCCESS)
    throw std::runtime_error("Failed to allocate command buffers!");

  for (size_t i = 0; i < command_buffers_.size(); i++)
  {
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(command_buffers_[i], &begin_info) != VK_SUCCESS)
      throw std::runtime_error("Failed to begin recording command buffer!");

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass_;
    render_pass_info.framebuffer = swapchain_framebuffers_[i];
    render_pass_info.renderArea.offset = { 0, 0 };
    render_pass_info.renderArea.extent = swapchain_extent_;

    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    clear_values[1].depthStencil = { 1.0f, 0 };

    render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(command_buffers_[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffers_[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_);

    VkBuffer vertexBuffers[] = { vertex_buffer_ };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(command_buffers_[i], 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(command_buffers_[i], index_buffer_, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(command_buffers_[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 0, 1, &descriptor_sets_[i], 0, nullptr);

    vkCmdDrawIndexed(command_buffers_[i], static_cast<uint32_t>(indices_.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(command_buffers_[i]);

    if (vkEndCommandBuffer(command_buffers_[i]) != VK_SUCCESS)
      throw std::runtime_error("Failed to record command buffer!");
  }
}

void VulkanEngine::CreateSyncObjects()
{
  image_available_semaphores_.resize(max_frames_in_flight_);
  render_finished_semaphores_.resize(max_frames_in_flight_);
  in_flight_fences_.resize(max_frames_in_flight_);
  images_in_flight_.resize(swapchain_images_.size(), VK_NULL_HANDLE);

  VkSemaphoreCreateInfo semaphore_info{};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fence_info{};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < max_frames_in_flight_; i++)
  {
    if (vkCreateSemaphore(device_, &semaphore_info, nullptr, &image_available_semaphores_[i]) != VK_SUCCESS ||
      vkCreateSemaphore(device_, &semaphore_info, nullptr, &render_finished_semaphores_[i]) != VK_SUCCESS ||
      vkCreateFence(device_, &fence_info, nullptr, &in_flight_fences_[i]) != VK_SUCCESS)
      throw std::runtime_error("Failed to create synchronization objects for a frame!");
  }
}
}
}
