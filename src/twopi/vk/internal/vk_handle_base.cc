#include <twopi/vk/internal/vk_handle_base.h>

namespace twopi
{
namespace vk
{
namespace internal
{
HandleBase::HandleBase()
{
}

HandleBase::~HandleBase()
{
}

void HandleBase::SetDependency(std::shared_ptr<HandleBase> dependency)
{
  dependency_ = dependency;
}
}
}
}
