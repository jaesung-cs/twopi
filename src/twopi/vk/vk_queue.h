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

  ~Queue();

  operator vk::Queue() const;

private:
  vk::Queue queue_;
};
}
}

#endif // TWOPI_VK_VK_QUEUE_H_
