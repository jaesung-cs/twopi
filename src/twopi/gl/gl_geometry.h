#ifndef TWOPI_GL_GL_GEOMETRY_H_
#define TWOPI_GL_GL_GEOMETRY_H_

#include <memory>
#include <vector>

namespace twopi
{
namespace gl
{
namespace impl
{
class GeometryImpl;
}

class Geometry
{
public:
  Geometry();
  ~Geometry();

  template <typename T>
  void SetAttribute(int location, int size, const std::vector<T>& attribute);

  template <typename T>
  void SetAttribute(int location, int size, std::initializer_list<T> attribute)
  {
    SetAttribute(location, size, std::vector<T>(attribute));
  }

  void SetElements(const std::vector<unsigned int>& elements);

  // Primitives
  void SetTriangles();
  void SetLines();

  void Draw();

private:
  std::unique_ptr<impl::GeometryImpl> impl_;
};
}
}

#endif // TWOPI_GL_GL_GEOMETRY_H_
