#include <twopi/vkw/vkw_queue.h>

#include <twopi/vkw/vkw_command_buffer.h>
#include <twopi/vkw/vkw_swapchain.h>
#include <twopi/vkw/vkw_semaphore.h>
#include <twopi/vkw/vkw_fence.h>

namespace twopi
{
namespace vkw
{
Queue::Queue()
{
}

Queue::Queue(vk::Queue queue)
  : queue_(queue)
{
}

Queue::Queue(vk::Queue queue, int queue_family_index)
  : queue_(queue), queue_family_index_(queue_family_index)
{
}

Queue::~Queue() = default;

Queue::operator vk::Queue() const
{
  return queue_;
}

void Queue::Submit(CommandBuffer command_buffer)
{
  Submit(command_buffer, {}, {}, {});
}

void Queue::Submit(CommandBuffer command_buffer, std::vector<Semaphore> wait_semaphores, std::vector<Semaphore> signal_semaphores, Fence fence)
{
  std::vector<vk::Semaphore> wait_semaphore_handles(wait_semaphores.cbegin(), wait_semaphores.cend());
  std::vector<vk::Semaphore> signal_semaphore_handles(signal_semaphores.cbegin(), signal_semaphores.cend());
  std::vector<vk::CommandBuffer> command_buffer_handles{ command_buffer };
  std::vector<vk::PipelineStageFlags> stage_mask(wait_semaphores.size(), vk::PipelineStageFlagBits::eColorAttachmentOutput);

  vk::SubmitInfo submit_info;
  submit_info
    .setWaitSemaphores(wait_semaphore_handles)
    .setWaitDstStageMask(stage_mask)
    .setSignalSemaphores(signal_semaphore_handles)
    .setCommandBuffers(command_buffer_handles);

  queue_.submit(submit_info, fence);
}

vk::Result Queue::Present(Swapchain swapchain, uint32_t image_index, std::vector<Semaphore> wait_semaphores)
{
  std::vector<vk::Semaphore> wait_semaphore_handles(wait_semaphores.cbegin(), wait_semaphores.cend());
  std::vector<vk::SwapchainKHR> swapchain_handles{ swapchain };
  std::vector<uint32_t> image_indices{ image_index };

  vk::PresentInfoKHR present_info{};
  present_info
    .setWaitSemaphores(wait_semaphore_handles)
    .setSwapchains(swapchain_handles)
    .setImageIndices(image_indices);

  return queue_.presentKHR(present_info);
}

void Queue::WaitIdle()
{
  queue_.waitIdle();
}
}
}
