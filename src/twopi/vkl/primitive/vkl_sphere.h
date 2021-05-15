#ifndef TWOPI_VKL_PRIMITIVE_VKL_SPHERE_H_
#define TWOPI_VKL_PRIMITIVE_VKL_SPHERE_H_

#include <vector>

namespace twopi
{
namespace vkl
{
class Sphere
{
public:
  Sphere() = delete;

  explicit Sphere(int segments);

  ~Sphere();

  const int NumVertices() const { return position_buffer_.size() / 3; }
  const int NumIndices() const { return index_buffer_.size(); }

  const auto& PositionBuffer() const { return position_buffer_; }
  const auto& NormalBuffer() const { return normal_buffer_; }
  const auto& IndexBuffer() const { return index_buffer_; }

private:
  std::vector<float> position_buffer_;
  std::vector<float> normal_buffer_;
  std::vector<uint32_t> index_buffer_;
};
}
}

#endif // TWOPI_VKL_PRIMITIVE_VKL_SPHERE_H_
