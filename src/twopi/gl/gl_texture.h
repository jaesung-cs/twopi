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
class Texture
{
public:
  Texture();
  ~Texture();

  template <typename T>
  void Load(std::shared_ptr<geometry::Image<T>> image);

  void Storage(int width, int height, int comp = 4);
  void StorageMultisample(int width, int height, int comp = 4);

  void Bind(int unit);

  unsigned int Id() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_GL_GL_TEXTURE_H_
