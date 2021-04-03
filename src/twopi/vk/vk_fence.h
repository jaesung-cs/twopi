#ifndef TWOPI_VK_VK_FENCE_H_
#define TWOPI_VK_VK_FENCE_H_

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Device;

class Fence
{
public:
  class Creator
  {
  public:
    Creator() = delete;
    explicit Creator(Device device);
    ~Creator();

    Fence Create();

  private:
    const vk::Device device_;

    vk::FenceCreateInfo create_info_{};
  };

public:
  Fence();
  Fence(vk::Device device, vk::Fence fence);

  ~Fence();

  void Destroy();

  operator vk::Fence() const;

  operator bool() const { return fence_; }

  void Wait();
  void Reset();

private:
  vk::Device device_;
  vk::Fence fence_;
};
}
}

#endif // TWOPI_VK_VK_FENCE_H_
