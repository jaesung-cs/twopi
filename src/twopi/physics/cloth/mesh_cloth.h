#ifndef TWOPI_CLOTH_MESH_CLOTH_H_
#define TWOPI_CLOTH_MESH_CLOTH_H_

#include <memory>

namespace twopi
{
namespace physics
{
namespace impl
{
class MeshClothImpl;
}

class MeshCloth
{
public:
  MeshCloth() = delete;

  explicit MeshCloth(int subdivision);

  ~MeshCloth();

private:
  std::unique_ptr<impl::MeshClothImpl> impl_;
};
}
}

#endif // TWOPI_CLOTH_MESH_CLOTH_H_
