#ifndef TWOPI_VKL_VKL_OBJECT_H_
#define TWOPI_VKL_VKL_OBJECT_H_

#include <twopi/vkl/vkl_context.h>

namespace twopi
{
namespace vkl
{
class Object
{
public:
  Object() = delete;

  Object(std::shared_ptr<vkl::Context> context);

  virtual ~Object();

protected:
  std::shared_ptr<vkl::Context> Context() const { return context_.lock(); }

private:
  std::weak_ptr<vkl::Context> context_;
};
}
}

#endif // TWOPI_VKL_VKL_OBJECT_H_
