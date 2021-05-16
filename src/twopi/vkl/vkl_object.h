#ifndef TWOPI_VKL_VKL_OBJECT_H_
#define TWOPI_VKL_VKL_OBJECT_H_

#include <memory>

namespace twopi
{
namespace vkl
{
class Context;

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
