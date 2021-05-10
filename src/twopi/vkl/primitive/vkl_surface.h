#ifndef TWOPI_VKL_PRIMITIVE_VKL_SURFACE_H_
#define TWOPI_VKL_PRIMITIVE_VKL_SURFACE_H_

#include <vector>

namespace twopi
{
namespace vkl
{
class Surface
{
public:
  Surface();

  ~Surface();

  const auto& PositionBuffer() const { return position_buffer_; }
  const auto& NormalBuffer() const { return normal_buffer_; }
  const auto& IndexBuffer() const { return index_buffer_; }

private:
  // p00, p10, p01, p11
  std::vector<float> position_buffer_;
  std::vector<float> normal_buffer_;
  std::vector<uint32_t> index_buffer_;
};
}
}

#endif // TWOPI_VKL_PRIMITIVE_VKL_SURFACE_H_
