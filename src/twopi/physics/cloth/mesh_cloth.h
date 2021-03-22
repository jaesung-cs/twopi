#ifndef TWOPI_CLOTH_MESH_CLOTH_H_
#define TWOPI_CLOTH_MESH_CLOTH_H_

#include <memory>

namespace twopi
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

#endif // TWOPI_CLOTH_MESH_CLOTH_H_
