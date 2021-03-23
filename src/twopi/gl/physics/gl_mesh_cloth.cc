#include <twopi/gl/physics/gl_mesh_cloth.h>

#include <vector>

namespace twopi
{
namespace gl
{
namespace physics
{
class MeshCloth::Impl
{
public:
  Impl() = delete;

  explicit Impl(int subdivision)
    : subdivision_(subdivision)
  {
    // TODO: create 2d texture
  }

  ~Impl() = default;

private:
  int subdivision_ = 1;
};

MeshCloth::MeshCloth(int subdivision)
{
  impl_ = std::make_unique<Impl>(subdivision);
}

MeshCloth::~MeshCloth() = default;
}
}
}
