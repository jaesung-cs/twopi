#ifndef TWOPI_VKW_VKW_PIPELINE_CACHE_H_
#define TWOPI_VKW_VKW_PIPELINE_CACHE_H_

#include <vector>

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Device;

class PipelineCache
{
public:
  class Creator
  {
  public:
    Creator() = delete;
    Creator(Device device);
    ~Creator();

    PipelineCache Create();

  private:
    const vk::Device device_;

    vk::PipelineCacheCreateInfo create_info_{};
  };

public:
  PipelineCache();
  PipelineCache(vk::Device device, vk::PipelineCache pipeline_cache);

  ~PipelineCache();
  
  void Destroy();

  operator vk::PipelineCache() const;

private:
  vk::Device device_;
  vk::PipelineCache pipeline_cache_;
};
}
}

#endif // TWOPI_VKW_VKW_PIPELINE_CACHE_H_
