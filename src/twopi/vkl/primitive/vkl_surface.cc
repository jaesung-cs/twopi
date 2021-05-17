#include <twopi/vkl/primitive/vkl_surface.h>

namespace twopi
{
namespace vkl
{
Surface::Surface()
{
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      position_buffer_.push_back((j - 1) * 2.f);
      position_buffer_.push_back(0.f);
      position_buffer_.push_back((i - 1) * 2.f);

      vx_buffer_.push_back(2.f);
      vx_buffer_.push_back(2.f);
      vx_buffer_.push_back(0.f);

      vy_buffer_.push_back(0.f);
      vy_buffer_.push_back(2.f);
      vy_buffer_.push_back(2.f);
    }
  }

  index_buffer_ = {
    0, 1, 3, 4,
    1, 2, 4, 5,
    3, 4, 6, 7,
    4, 5, 7, 8,
  };
}

Surface::~Surface() = default;
}
}
