#include <twopi/vkl/primitive/vkl_halfsphere_surface.h>

namespace twopi
{
namespace vkl
{
HalfsphereSurface::HalfsphereSurface()
{
  std::vector<float> xs = { 1.f, 0.f, -1.f, 0.f };
  std::vector<float> ys = { 0.f, 1.f, 0.f, -1.f };

  constexpr float s = 0.70710678118654752440084436210485f;

  for (int i = 0; i < 4; i++)
  {
    const auto x = xs[i];
    const auto y = ys[i];

    position_buffer_.push_back(x);
    position_buffer_.push_back(y);
    position_buffer_.push_back(0.f);
    normal_buffer_.push_back(x);
    normal_buffer_.push_back(y);
    normal_buffer_.push_back(0.f);

    position_buffer_.push_back(x * s);
    position_buffer_.push_back(y * s);
    position_buffer_.push_back(s);
    normal_buffer_.push_back(x * s);
    normal_buffer_.push_back(y * s);
    normal_buffer_.push_back(s);
  }

  index_buffer_ = {
    // Circle
    1, 0, 3, 2,
    3, 2, 5, 4,
    5, 4, 7, 6,
    7, 6, 1, 0,
    // Top
    1, 3, 7, 5,
  };
}

HalfsphereSurface::~HalfsphereSurface() = default;
}
}
