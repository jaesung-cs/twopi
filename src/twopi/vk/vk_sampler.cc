#include <twopi/vk/vk_sampler.h>

#include <twopi/vk/vk_device.h>
#include <twopi/vk/vk_physical_device.h>

namespace twopi
{
namespace vkw
{
//
// Creator
//
Sampler::Creator::Creator(Device device)
  : device_(device)
{
}

Sampler::Creator::~Creator() = default;

Sampler::Creator& Sampler::Creator::EnableAnisotropy(PhysicalDevice physical_device)
{
  create_info_
    .setAnisotropyEnable(VK_TRUE)
    .setMaxAnisotropy(physical_device.Properties().limits.maxSamplerAnisotropy);

  return *this;
}

Sampler Sampler::Creator::Create()
{
  create_info_
    .setMagFilter(vk::Filter::eLinear)
    .setMinFilter(vk::Filter::eLinear)
    .setAddressModeU(vk::SamplerAddressMode::eRepeat)
    .setAddressModeV(vk::SamplerAddressMode::eRepeat)
    .setAddressModeW(vk::SamplerAddressMode::eRepeat)
    .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
    .setUnnormalizedCoordinates(VK_FALSE)
    .setCompareEnable(VK_FALSE)
    .setCompareOp(vk::CompareOp::eAlways)
    .setMipmapMode(vk::SamplerMipmapMode::eLinear)
    .setMipLodBias(0.f)
    .setMinLod(0.f)
    .setMaxLod(0.f);

  const auto sampler = device_.createSampler(create_info_);
  return Sampler{ device_, sampler };
}

//
// Sampler
//
Sampler::Sampler()
{
}

Sampler::Sampler(vk::Device device, vk::Sampler sampler)
  : device_(device), sampler_(sampler)
{
}

Sampler::~Sampler() = default;

void Sampler::Destroy()
{
  device_.destroySampler(sampler_);
}

Sampler::operator vk::Sampler() const
{
  return sampler_;
}
}
}
