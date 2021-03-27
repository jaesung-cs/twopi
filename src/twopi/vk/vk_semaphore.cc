#include <twopi/vk/vk_semaphore.h>

#include <twopi/vk/vk_device.h>

namespace twopi
{
namespace vkw
{
//
// Creator
//
Semaphore::Creator::Creator(Device device)
  : device_(device)
{
}

Semaphore::Creator::~Creator() = default;

Semaphore Semaphore::Creator::Create()
{
  const auto handle = device_.createSemaphore(create_info_);
  return Semaphore{ device_, handle };
}

//
// Semaphore
//
Semaphore::Semaphore()
{
}

Semaphore::Semaphore(vk::Device device, vk::Semaphore semaphore)
  : device_(device), semaphore_(semaphore)
{
}

Semaphore::~Semaphore() = default;

void Semaphore::Destroy()
{
  device_.destroySemaphore(semaphore_);
}

Semaphore::operator vk::Semaphore() const
{
  return semaphore_;
}
}
}
