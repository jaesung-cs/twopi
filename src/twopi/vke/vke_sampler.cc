#include <twopi/vke/vke_sampler.h>

#include <twopi/vke/vke_context.h>

namespace twopi
{
namespace vke
{
Sampler::Sampler(std::shared_ptr<vke::Context> context, const std::string& filepath)
  : context_(context)
{
  vk::SamplerCreateInfo sampler_create_info;
  sampler_create_info
    .setAnisotropyEnable(true)
    .setMaxAnisotropy(context->PhysicalDevice().getProperties().limits.maxSamplerAnisotropy)
    .setMagFilter(vk::Filter::eLinear)
    .setMinFilter(vk::Filter::eLinear)
    .setAddressModeU(vk::SamplerAddressMode::eRepeat)
    .setAddressModeV(vk::SamplerAddressMode::eRepeat)
    .setAddressModeW(vk::SamplerAddressMode::eRepeat)
    .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
    .setUnnormalizedCoordinates(false)
    .setCompareEnable(false)
    .setCompareOp(vk::CompareOp::eAlways)
    .setMipmapMode(vk::SamplerMipmapMode::eLinear)
    .setMipLodBias(0.f)
    .setMinLod(0.f)
    .setMaxLod(mip_level_);

  sampler_ = context->Device().createSampler(sampler_create_info);
}

Sampler::~Sampler()
{
  Context()->Device().destroySampler(sampler_);
}

std::shared_ptr<Context> Sampler::Context() const
{
  return context_.lock();
}
}
}
