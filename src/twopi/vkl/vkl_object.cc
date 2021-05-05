#include <twopi/vkl/vkl_object.h>

namespace twopi
{
namespace vkl
{
Object::Object(std::shared_ptr<vkl::Context> context)
  : context_{ context }
{
}

Object::~Object() = default;
}
}
