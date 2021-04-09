#ifndef TWOPI_VKE_VKE_SAMPLER_H_
#define TWOPI_VKE_VKE_SAMPLER_H_

#include <memory>

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vke
{
class Context;

class Sampler
{
public:
  Sampler() = delete;

  Sampler(std::shared_ptr<Context> context, const std::string& filepath);

  ~Sampler();

private:
  std::shared_ptr<Context> Context() const;

  std::weak_ptr<vke::Context> context_;

  vk::Sampler sampler_;
  const int mip_level_ = 3;
};
}
}

#endif // TWOPI_VKE_VKE_SAMPLER_H_
