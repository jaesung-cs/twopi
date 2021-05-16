#ifndef TWOPI_VKL_MODEL_VKL_CUBESKIN_H_
#define TWOPI_VKL_MODEL_VKL_CUBESKIN_H_

#include <array>

#include <twopi/vkl/vkl_object.h>
#include <twopi/vkl/vkl_memory.h>

namespace twopi
{
namespace vkl
{
class Cubeskin : public Object
{
public:
  Cubeskin() = delete;

  Cubeskin(std::shared_ptr<vkl::Context> context, int segments, int depth);

  ~Cubeskin();

  const vk::Buffer StorageBuffer() const { return shell_buffer_; }
  uint32_t StorageDoubleBufferOffset() { return shell_offset_; }
  uint32_t StorageBufferSize() const { return shell_buffer_size_; }

  void Update(vk::CommandBuffer& command_buffer);
  void DrawSupports(vk::CommandBuffer& command_buffer);

private:
  const int segments_;
  const int depth_;

  // Vertex swap buffer
  vk::Buffer shell_buffer_;
  Memory shell_memory_;
  uint32_t shell_offset_ = 0;
  uint32_t shell_buffer_size_ = 0;

  // Multiple index buffers
  vk::Buffer index_buffer_;
  Memory index_memory_;
  vk::DeviceSize support_index_offset_ = 0;
  uint32_t num_support_indices_ = 0;
};
}
}

#endif // TWOPI_VKL_MODEL_VKL_CUBESKIN_H_
