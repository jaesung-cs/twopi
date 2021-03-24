#include <twopi/vk/internal/vk_instance_handle.h>

#include <iostream>

namespace twopi
{
namespace vk
{
namespace internal
{
InstanceHandle::InstanceHandle(VkInstance instance)
  : Handle(instance)
{
}

InstanceHandle::~InstanceHandle()
{
  Destroy();
}

void InstanceHandle::Destroy()
{
  std::cout << "Destroy Instance" << std::endl;
  vkDestroyInstance(*this, nullptr);
}
}
}
}
