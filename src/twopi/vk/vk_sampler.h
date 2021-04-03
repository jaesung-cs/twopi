#ifndef TWOPI_VK_VK_SAMPLER_H_
#define TWOPI_VK_VK_SAMPLER_H_

#include <vector>

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Device;
class PhysicalDevice;

class Sampler
{
public:
  class Creator
  {
  public:
    Creator() = delete;
    explicit Creator(Device device);
    ~Creator();

    Creator& SetMipLevels(int mip_levels);
    Creator& EnableAnisotropy(PhysicalDevice physical_device);

    Sampler Create();

  private:
    vk::Device device_;
    vk::SamplerCreateInfo create_info_;
    int mip_levels_ = 0;
  };

public:
  Sampler();
  Sampler(vk::Device device, vk::Sampler sampler);

  ~Sampler();

  operator vk::Sampler() const;

  void Destroy();

private:
  vk::Device device_;
  vk::Sampler sampler_;
};
}
}

#endif // TWOPI_VK_VK_SAMPLER_H_
