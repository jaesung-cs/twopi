#include <twopi/vkl/primitive/vkl_surface.h>

namespace twopi
{
namespace vkl
{
Surface::Surface()
{
  constexpr float nz = 0.86602540378443864676372317075294f;

  position_buffer_.push_back(-1.f);
  position_buffer_.push_back(-1.f);
  position_buffer_.push_back(0.5f);
  normal_buffer_.push_back(-0.5f);
  normal_buffer_.push_back(-0.5f);
  normal_buffer_.push_back(nz);

  position_buffer_.push_back(1.f);
  position_buffer_.push_back(-1.f);
  position_buffer_.push_back(0.5f);
  normal_buffer_.push_back(0.5f);
  normal_buffer_.push_back(-0.5f);
  normal_buffer_.push_back(nz);

  position_buffer_.push_back(-1.f);
  position_buffer_.push_back(1.f);
  position_buffer_.push_back(0.5f);
  normal_buffer_.push_back(-0.5f);
  normal_buffer_.push_back(0.5f);
  normal_buffer_.push_back(nz);

  position_buffer_.push_back(1.f);
  position_buffer_.push_back(1.f);
  position_buffer_.push_back(0.5f);
  normal_buffer_.push_back(0.5f);
  normal_buffer_.push_back(0.5f);
  normal_buffer_.push_back(nz);

  index_buffer_ = {
    0, 1, 2, 3
  };
}

Surface::~Surface() = default;
}
}
