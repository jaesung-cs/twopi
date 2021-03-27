#ifndef TWOPI_VK_VK_QUEUE_H_
#define TWOPI_VK_VK_QUEUE_H_

#include <vector>

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Semaphore;
class CommandBuffer;
class Swapchain;
class Fence;

class Queue
{
public:
  Queue();
  Queue(vk::Queue queue);
  Queue(vk::Queue queue, int queue_family_index);

  ~Queue();

  operator vk::Queue() const;

  auto QueueFamilyIndex() const { return queue_family_index_; }

  void Submit(CommandBuffer command_buffer, std::vector<Semaphore> wait_semaphores, std::vector<Semaphore> signal_semaphores, Fence fence);
  vk::Result Present(Swapchain swapchain, uint32_t image_index, std::vector<Semaphore> wait_semaphores);

private:
  vk::Queue queue_;
  int queue_family_index_ = -1;
};
}
}

#endif // TWOPI_VK_VK_QUEUE_H_
