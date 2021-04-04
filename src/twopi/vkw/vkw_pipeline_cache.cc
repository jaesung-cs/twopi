#include <twopi/vkw/vkw_pipeline_cache.h>

#include <twopi/vkw/vkw_device.h>
#include <twopi/vkw/vkw_descriptor_set_layout.h>

namespace twopi
{
namespace vkw
{
//
// Creator
//
PipelineCache::Creator::Creator(Device device)
  : device_(device)
{
}

PipelineCache::Creator::~Creator() = default;

PipelineCache PipelineCache::Creator::Create()
{
  auto handle = device_.createPipelineCache(create_info_);
  return PipelineCache{ device_, handle };
}

//
// PipelineCache
//
PipelineCache::PipelineCache()
{
}

PipelineCache::PipelineCache(vk::Device device, vk::PipelineCache pipeline_cache)
  : device_(device), pipeline_cache_(pipeline_cache)
{
}

PipelineCache::~PipelineCache() = default;

void PipelineCache::Destroy()
{
  device_.destroyPipelineCache(pipeline_cache_);
}

PipelineCache::operator vk::PipelineCache() const
{
  return pipeline_cache_;
}
}
}
