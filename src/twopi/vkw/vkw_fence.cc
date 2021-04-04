#include <twopi/vkw/vkw_fence.h>

#include <twopi/vkw/vkw_device.h>

namespace twopi
{
namespace vkw
{
//
// Creator
//
Fence::Creator::Creator(Device device)
  : device_(device)
{
  create_info_.setFlags(vk::FenceCreateFlagBits::eSignaled);
}

Fence::Creator::~Creator() = default;

Fence Fence::Creator::Create()
{
  const auto handle = device_.createFence(create_info_);
  return Fence{ device_, handle };
}

//
// Fence
//
Fence::Fence()
{
}

Fence::Fence(vk::Device device, vk::Fence fence)
  : device_(device), fence_(fence)
{
}

Fence::~Fence() = default;

void Fence::Destroy()
{
  device_.destroyFence(fence_);
}

Fence::operator vk::Fence() const
{
  return fence_;
}

void Fence::Wait()
{
  auto result = device_.waitForFences({ fence_ }, true, UINT64_MAX);
  // TODO: handle with result
}

void Fence::Reset()
{
  device_.resetFences({ fence_ });
}
}
}
