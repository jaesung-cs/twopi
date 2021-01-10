#ifndef TWOPI_VK_VULKAN_ENGINE_H_
#define TWOPI_VK_VULKAN_ENGINE_H_

#include <vector>
#include <optional>
#include <array>
#include <string>
#include <functional>

#include <Windows.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace twopi
{
namespace vk
{
class VulkanEngine
{
public:
  struct PreparationOptions
  {
    // Windows window handles for creating surface
    HINSTANCE hInstance = nullptr;
    HWND hWindow = nullptr;

    // Layers and extensions
    bool enable_validation_layers = false;
    std::vector<std::string> instance_extensions;
    std::vector<std::string> device_extensions;

    // [optional] Physical device selector function
    std::function<VkPhysicalDevice(VkInstance)> physical_device_selector;
  };

private:
  struct QueueFamilyIndices
  {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    bool IsComplete() const
    {
      return graphics_family.has_value() && present_family.has_value();
    }
  };

  struct SwapchainSupportDetails
  {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
  };

  struct Vertex
  {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription GetBindingDescription()
    {
      VkVertexInputBindingDescription binding_description{};
      binding_description.binding = 0;
      binding_description.stride = sizeof(Vertex);
      binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return binding_description;
    }

    static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
    {
      std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions{};

      attribute_descriptions[0].binding = 0;
      attribute_descriptions[0].location = 0;
      attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_descriptions[0].offset = offsetof(Vertex, pos);

      attribute_descriptions[1].binding = 0;
      attribute_descriptions[1].location = 1;
      attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_descriptions[1].offset = offsetof(Vertex, color);

      attribute_descriptions[2].binding = 0;
      attribute_descriptions[2].location = 2;
      attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
      attribute_descriptions[2].offset = offsetof(Vertex, texCoord);

      return attribute_descriptions;
    }

    bool operator == (const Vertex& other) const
    {
      return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }

    struct Hash
    {
      size_t operator () (twopi::vk::VulkanEngine::Vertex const& vertex) const
      {
        return ((std::hash<glm::vec3>()(vertex.pos) ^ (std::hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (std::hash<glm::vec2>()(vertex.texCoord) << 1);
      }
    };
  };

  struct UniformBufferObject
  {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
  };

public:
  VulkanEngine();
  ~VulkanEngine();

  void Prepare(const PreparationOptions& preparation_params);

  auto GraphicsQueue() const { return graphics_queue_; }
  auto Device() const { return device_; }

  void Draw();

  void DeviceWaitIdle();

  void Cleanup();

private:
  void UpdateUniformBuffer(uint32_t current_image);

  void RecreateSwapchain();
  void CleanupSwapchain();

  void CreateInstance(std::vector<std::string> extensions);
  std::vector<const char*> GetRequiredExtensions();
  void SetupDebugMessenger();
  void CreateSurface(const HINSTANCE hInstance, const HWND hWindow);

  void PickPhysicalDevice(std::function<VkPhysicalDevice(VkInstance)> physical_device_selector);
  void PickBestPhysicalDevice();
  bool IsDeviceSuitable(VkPhysicalDevice device);
  QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
  bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
  SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device);
  VkSampleCountFlagBits GetMaxUsableSampleCount();

  void CreateLogicalDevice();

  void CreateSwapchain();
  VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
  VkPresentModeKHR ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
  VkExtent2D ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities);

  void CreateSwapchainImageViews();
  VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels);

  void CreateRenderPass();
  VkFormat FindDepthFormat();
  VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

  void CreateDescriptorSetLayout();

  void CreateGraphicsPipeline();
  VkShaderModule CreateShaderModule(const std::vector<char>& code);

  void CreateCommandPool();

  void CreateColorResources();
  void CreateDepthResources();
  void CreateImage(uint32_t width, uint32_t height, uint32_t mip_levels, VkSampleCountFlagBits num_samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& image_memory);
  uint32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);

  void CreateFramebuffers();

  void CreateTextureImage(const std::string& texture_path);
  void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory);
  void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels);
  VkCommandBuffer BeginSingleTimeCommands();
  void EndSingleTimeCommands(VkCommandBuffer command_buffer);
  void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
  void GenerateMipmaps(VkImage image, VkFormat image_format, int32_t tex_width, int32_t tex_height, uint32_t mip_levels);

  void CreateTextureImageView();
  void CreateTextureSampler();

  void LoadModel(const std::string& obj_filename);

  void CreateVertexBuffer();
  void CreateIndexBuffer();
  void CopyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);

  void CreateUniformBuffers();

  void CreateDescriptorPool();
  void CreateDescriptorSets();

  void CreateCommandBuffers();

  void CreateSyncObjects();

  bool enable_validation_layers_ = false;

  VkInstance instance_ = nullptr;
  VkDebugUtilsMessengerEXT debug_messenger_ = nullptr;
  VkSurfaceKHR surface_ = nullptr;

  VkPhysicalDevice physical_device_ = nullptr;
  VkSampleCountFlagBits msaa_samples_ = VK_SAMPLE_COUNT_1_BIT;

  VkDevice device_ = nullptr;
  VkQueue graphics_queue_ = nullptr;
  VkQueue present_queue_ = nullptr;

  VkSwapchainKHR swapchain_ = nullptr;
  VkExtent2D swapchain_extent_{};
  VkFormat swapchain_image_format_ = VkFormat::VK_FORMAT_UNDEFINED;
  std::vector<VkImage> swapchain_images_;

  std::vector<VkImageView> swapchain_image_views_;

  VkRenderPass render_pass_ = nullptr;

  VkDescriptorSetLayout descriptor_set_layout_ = nullptr;

  VkPipelineLayout pipeline_layout_ = nullptr;
  VkPipeline graphics_pipeline_ = nullptr;

  VkCommandPool command_pool_ = nullptr;

  VkImage color_image_ = nullptr;
  VkDeviceMemory color_image_memory_ = nullptr;
  VkImageView color_image_view_ = nullptr;

  VkImage depth_image_ = nullptr;
  VkDeviceMemory depth_image_memory_ = nullptr;
  VkImageView depth_image_view_ = nullptr;

  std::vector<VkFramebuffer> swapchain_framebuffers_;

  uint32_t mip_levels_ = 0;
  VkImage texture_image_ = nullptr;
  VkDeviceMemory texture_image_memory_ = nullptr;
  VkImageView texture_image_view_ = nullptr;
  VkSampler texture_sampler_ = nullptr;

  // Model
  std::vector<Vertex> vertices_;
  std::vector<uint32_t> indices_;

  VkBuffer vertex_buffer_ = nullptr;
  VkDeviceMemory vertex_buffer_memory_ = nullptr;

  VkBuffer index_buffer_ = nullptr;
  VkDeviceMemory index_buffer_memory_ = nullptr;

  std::vector<VkBuffer> uniform_buffers_;
  std::vector<VkDeviceMemory> uniform_buffers_memory_;

  VkDescriptorPool descriptor_pool_ = nullptr;
  std::vector<VkDescriptorSet> descriptor_sets_;

  std::vector<VkCommandBuffer> command_buffers_;

  static constexpr int max_frames_in_flight_ = 2;
  std::vector<VkSemaphore> image_available_semaphores_;
  std::vector<VkSemaphore> render_finished_semaphores_;
  std::vector<VkFence> in_flight_fences_;
  std::vector<VkFence> images_in_flight_;
  size_t current_frame_ = 0;

  bool framebuffer_resized_ = false;

  // Window
  HWND hWindow_ = nullptr;
};
}
}

#endif // TWOPI_VK_VULKAN_ENGINE_H_
