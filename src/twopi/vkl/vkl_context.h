#ifndef TWOPI_VKL_VKL_CONTEXT_H_
#define TWOPI_VKL_VKL_CONTEXT_H_

#include <optional>
#include <vector>

#include <vulkan/vulkan.hpp>

struct GLFWwindow;

namespace twopi
{
namespace vkl
{
struct Memory;
class MemoryManager;

class Context
{
public:
  Context(GLFWwindow* glfw_window);

  ~Context();

  auto PhysicalDevice() const { return physical_device_; }
  auto Device() const { return device_; }
  auto Queue() const { return queue_; }
  auto PresentQueue() const { return present_queue_; }
  auto Surface() const { return surface_; }

  std::vector<uint32_t> QueueFamilyIndices() const;

  [[nodiscard]] Memory AllocateDeviceMemory(vk::Buffer buffer);
  [[nodiscard]] Memory AllocateDeviceMemory(vk::Image image);
  [[nodiscard]] Memory AllocateHostMemory(vk::Buffer buffer);
  [[nodiscard]] Memory AllocatePersistentlyMappedMemory(vk::Buffer buffer);

  std::vector<vk::CommandBuffer> AllocateCommandBuffers(int count);
  std::vector<vk::CommandBuffer> AllocateTransientCommandBuffers(int count);

  void FreeCommandBuffers(std::vector<vk::CommandBuffer>&& command_buffers);
  void FreeTransientCommandBuffers(std::vector<vk::CommandBuffer>&& command_buffers);

  template <typename T>
  Context& ToGpu(const std::vector<T>& data, vk::Buffer buffer, int offset)
  {
    // TODO: parallelize copy commands
    auto command_buffer = AllocateTransientCommandBuffers(1)[0];

    return *this;
  }

private:
  void CreateInstance(GLFWwindow* glfw_window);
  void DestroyInstance();

  void CreateDevice();
  void DestroyDevice();

  void CreateCommandPools();
  void DestroyCommandPools();

  vk::Instance instance_;
  vk::DebugUtilsMessengerEXT messenger_;
  vk::SurfaceKHR surface_;

  vk::PhysicalDevice physical_device_;
  vk::Device device_;
  vk::Queue queue_;
  vk::Queue present_queue_;

  std::optional<uint32_t> queue_index_;

  std::unique_ptr<MemoryManager> memory_manager_;

  // Stage buffer
  std::unique_ptr<StageBuffer> stage_buffer_;
  void* stage_buffer_map_;
  vk::Semaphore stage_buffer_semaphore_;

  // Command pool
  vk::CommandPool command_pool_;
  vk::CommandPool transient_command_pool_;
};
}
}

#endif // TWOPI_VKL_VKL_CONTEXT_H_
