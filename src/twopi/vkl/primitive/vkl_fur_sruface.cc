#include <twopi/vkl/primitive/vkl_fur_surface.h>

namespace twopi
{
namespace vkl
{
FurSurface::FurSurface()
{
  position_buffer_.push_back(-3.f);
  position_buffer_.push_back(-3.f);
  position_buffer_.push_back(1.f);
  normal_buffer_.push_back(0.f);
  normal_buffer_.push_back(0.f);
  normal_buffer_.push_back(1.f);

  position_buffer_.push_back(3.f);
  position_buffer_.push_back(-3.f);
  position_buffer_.push_back(1.f);
  normal_buffer_.push_back(0.f);
  normal_buffer_.push_back(0.f);
  normal_buffer_.push_back(1.f);

  position_buffer_.push_back(-3.f);
  position_buffer_.push_back(3.f);
  position_buffer_.push_back(1.f);
  normal_buffer_.push_back(0.f);
  normal_buffer_.push_back(0.f);
  normal_buffer_.push_back(1.f);

  position_buffer_.push_back(3.f);
  position_buffer_.push_back(3.f);
  position_buffer_.push_back(1.f);
  normal_buffer_.push_back(0.f);
  normal_buffer_.push_back(0.f);
  normal_buffer_.push_back(1.f);

  index_buffer_ = {
    // Circle
    0, 1, 2, 3
  };
}

FurSurface::~FurSurface() = default;
}
}
