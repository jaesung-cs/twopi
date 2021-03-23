#ifndef TWOPI_GL_PHYSICS_MESH_CLOTH_H_
#define TWOPI_GL_PHYSICS_MESH_CLOTH_H_

#include <memory>

namespace twopi
{
namespace gl
{
namespace physics
{
class MeshCloth
{
public:
  MeshCloth() = delete;

  explicit MeshCloth(int subdivision);

  ~MeshCloth();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}
}

#endif // TWOPI_GL_PHYSICS_MESH_CLOTH_H_
