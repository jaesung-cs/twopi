#include <twopi/scene/geometry/patch_geometry.h>

namespace twopi
{
namespace scene
{
class PatchGeometry::Impl
{
public:
  Impl()
  {
  }

  ~Impl()
  {
  }

private:
};

PatchGeometry::PatchGeometry()
  : Geometry()
{
  impl_ = std::make_unique<Impl>();
}

PatchGeometry::~PatchGeometry() = default;
}
}
