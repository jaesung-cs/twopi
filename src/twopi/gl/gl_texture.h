#ifndef TWOPI_GL_GL_TEXTURE_H_
#define TWOPI_GL_GL_TEXTURE_H_

#include <memory>
#include <vector>

namespace twopi
{
namespace geometry
{
template <typename T>
class Image;
}

namespace gl
{
namespace impl
{
class TextureImpl;
}

class Texture
{
public:
  Texture();
  ~Texture();

  template <typename T>
  void Load(std::shared_ptr<geometry::Image<T>> image);

  void Bind(int unit);

private:
  std::unique_ptr<impl::TextureImpl> impl_;
};
}
}

#endif // TWOPI_GL_GL_TEXTURE_H_
