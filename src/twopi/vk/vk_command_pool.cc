#include <twopi/vk/vk_command_pool.h>

#include <twopi/vk/vk_device.h>
#include <twopi/vk/vk_queue.h>

namespace twopi
{
namespace vkw
{
//
// Creator
//
CommandPool::Creator::Creator(Device device)
  : device_(device)
{
}

CommandPool::Creator::~Creator() = default;

CommandPool::Creator& CommandPool::Creator::SetQueue(Queue queue)
{
  create_info_.setQueueFamilyIndex(queue.QueueFamilyIndex());
  return *this;
}

CommandPool CommandPool::Creator::Create()
{
  const auto handle = device_.createCommandPool(create_info_);
  return CommandPool{ device_, handle };
}

//
// CommandPool
//
CommandPool::CommandPool()
{
}

CommandPool::CommandPool(vk::Device device, vk::CommandPool command_pool)
  : device_(device), command_pool_(command_pool)
{
}

CommandPool::~CommandPool() = default;

void CommandPool::Destroy()
{
  device_.destroyCommandPool(command_pool_);
}

CommandPool::operator vk::CommandPool() const
{
  return command_pool_;
}
}
}
