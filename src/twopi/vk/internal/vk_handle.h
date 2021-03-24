#ifndef TWOPI_VK_INTERNAL_VK_HANDLE_H_
#define TWOPI_VK_INTERNAL_VK_HANDLE_H_

#include <vector>
#include <memory>

#include <twopi/vk/internal/vk_handle_base.h>

namespace twopi
{
namespace vk
{
namespace internal
{
template <typename HandleType>
class Handle : public HandleBase
{
public:
  Handle() = delete;

  Handle(HandleType handle)
    : handle_(handle)
  {
  }

  virtual ~Handle() override = default;

  operator HandleType() const { return handle_; }

private:
  HandleType handle_ = nullptr;
};

template <typename ParentType, typename HandleType>
class DependentHandle : public HandleBase
{
public:
  DependentHandle() = delete;

  DependentHandle(HandleType handle)
    : HandleBase(), handle_(handle)
  {
  }

  DependentHandle(ParentType parent, HandleType handle)
    : HandleBase(), parent_(parent), handle_(handle)
  {
  }

  virtual ~DependentHandle() override = default;

  operator HandleType() const { return handle_; }

  ParentType Parent() const { return parent_; }

private:
  ParentType parent_ = nullptr;
  HandleType handle_ = nullptr;
};
}
}
}

#endif // TWOPI_VK_INTERNAL_VK_HANDLE_H_
