#ifndef TWOPI_VK_INTERNAL_VK_HANDLE_BASE_H_
#define TWOPI_VK_INTERNAL_VK_HANDLE_BASE_H_

#include <vector>
#include <memory>

namespace twopi
{
namespace vk
{
namespace internal
{
class HandleBase
{
public:
  HandleBase();
  virtual ~HandleBase();

  virtual void Destroy() = 0;

  void SetDependency(std::shared_ptr<HandleBase> dependency);

private:
  std::shared_ptr<HandleBase> dependency_;
};
}
}
}

#endif // TWOPI_VK_INTERNAL_VK_HANDLE_BASE_H_
