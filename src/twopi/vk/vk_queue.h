#ifndef TWOPI_VK_VK_QUEUE_H_
#define TWOPI_VK_VK_QUEUE_H_

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Queue
{
public:
  Queue();
  Queue(vk::Queue queue);
  Queue(vk::Queue queue, int queue_family_index);

  ~Queue();

  operator vk::Queue() const;

  auto QueueFamilyIndex() const { return queue_family_index_; }

private:
  vk::Queue queue_;
  int queue_family_index_ = -1;
};
}
}

#endif // TWOPI_VK_VK_QUEUE_H_
