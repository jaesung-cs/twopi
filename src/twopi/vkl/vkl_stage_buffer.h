#ifndef TWOPI_VKL_VKL_STAGE_BUFFER_H_
#define TWOPI_VKL_VKL_STAGE_BUFFER_H_

#include <twopi/vkl/vkl_object.h>
#include <twopi/vkl/vkl_memory.h>

namespace twopi
{
namespace vkl
{
class StageBuffer : public Object
{
public:
  StageBuffer() = delete;

  StageBuffer(std::shared_ptr<vkl::Context> context);

  ~StageBuffer() override;

  operator void* const () const;
  operator void* ();

private:
  vk::Buffer buffer_;
  Memory memory_;
  void* map_;
};
}
}

#endif // TWOPI_VKL_VKL_STAGE_BUFFER_H_
