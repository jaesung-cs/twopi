#include <twopi/vk/vk_queue.h>

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

Queue::~Queue() = default;

Queue::operator vk::Queue() const
{
  return queue_;
}
}
}
