#include <twopi/vkl/primitive/vkl_surface.h>

namespace twopi
{
namespace vkl
{
Surface::Surface()
{
  position_buffer_.push_back(-1.f);
  position_buffer_.push_back(-1.f);
  position_buffer_.push_back(0.f);

  position_buffer_.push_back(1.f);
  position_buffer_.push_back(-1.f);
  position_buffer_.push_back(0.f);

  position_buffer_.push_back(-1.f);
  position_buffer_.push_back(1.f);
  position_buffer_.push_back(0.f);

  position_buffer_.push_back(1.f);
  position_buffer_.push_back(1.f);
  position_buffer_.push_back(0.f);

  for (int i = 0; i < 4; i++)
  {
    vx_buffer_.push_back(2.f);
    vx_buffer_.push_back(0.f);
    vx_buffer_.push_back(2.f);

    vy_buffer_.push_back(0.f);
    vy_buffer_.push_back(2.f);
    vy_buffer_.push_back(2.f);
  }

  index_buffer_ = {
    0, 1, 2, 3
  };
}

Surface::~Surface() = default;
}
}
