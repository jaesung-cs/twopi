#include <twopi/vkl/primitive/vkl_floor.h>

namespace twopi
{
namespace vkl
{
Floor::Floor(float range)
{
  position_buffer_ = {
    -range, -range, 0.f,
    range, -range, 0.f,
    -range, range, 0.f,
    range, range, 0.f,
  };

  normal_buffer_ = {
    0.f, 0.f, 1.f,
    0.f, 0.f, 1.f,
    0.f, 0.f, 1.f,
    0.f, 0.f, 1.f,
  };

  tex_coord_buffer_ = {
    -range, -range,
    range, -range,
    -range, range,
    range, range,
  };

  index_buffer_ = {
    0, 1, 2, 2, 1, 3
  };
}

Floor::~Floor() = default;
}
}
