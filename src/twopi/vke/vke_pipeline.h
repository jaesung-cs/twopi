#ifndef TWOPI_VKE_VKE_PIPELINE_H_
#define TWOPI_VKE_VKE_PIPELINE_H_

#include <string>
#include <memory>
#include <vector>

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vke
{
class Context;
class Swapchain;
struct Attribute;
struct Uniform;

class Pipeline
{
public:
  Pipeline() = delete;

  Pipeline(std::shared_ptr<Context> context,
    std::shared_ptr<Swapchain> swapchain,
    const std::string& dirpath, const std::string& shader_name,
    const std::vector<Attribute>& attributes,
    const std::vector<Uniform>& uniforms);

  ~Pipeline();

private:
  std::shared_ptr<Context> Context() const;

  std::weak_ptr<vke::Context> context_;

  vk::Pipeline graphics_pipeline_;
  vk::PipelineLayout pipeline_layout_;
};
}
}

#endif // TWOPI_VKE_VKE_PIPELINE_H_
