#ifndef TWOPI_VKL_PRIMITIVE_VKL_FLOOR_H_
#define TWOPI_VKL_PRIMITIVE_VKL_FLOOR_H_

#include <vector>

namespace twopi
{
namespace vkl
{
class Floor
{
public:
  Floor() = delete;

  explicit Floor(float range);

  ~Floor();

  const auto& PositionBuffer() const { return position_buffer_; }
  const auto& NormalBuffer() const { return normal_buffer_; }
  const auto& TexCoordBuffer() const { return tex_coord_buffer_; }
  const auto& IndexBuffer() const { return index_buffer_; }

private:
  std::vector<float> position_buffer_;
  std::vector<float> normal_buffer_;
  std::vector<float> tex_coord_buffer_;
  std::vector<uint32_t> index_buffer_;
};
}
}

#endif // TWOPI_VKL_PRIMITIVE_VKL_FLOOR_H_
