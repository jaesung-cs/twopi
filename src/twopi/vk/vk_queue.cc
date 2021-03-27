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

Queue::Queue(vk::Queue queue, int queue_family_index)
  : queue_(queue), queue_family_index_(queue_family_index)
{
}

Queue::~Queue() = default;

Queue::operator vk::Queue() const
{
  return queue_;
}
}
}
