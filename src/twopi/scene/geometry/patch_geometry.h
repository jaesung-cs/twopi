#ifndef TWOPI_SCENE_GEOMETRY_PATCH_GEOMETRY_H_
#define TWOPI_SCENE_GEOMETRY_PATCH_GEOMETRY_H_

#include <twopi/scene/geometry/geometry.h>

namespace twopi
{
namespace scene
{
class PatchGeometry : public Geometry
{
public:
  PatchGeometry();
  ~PatchGeometry() override;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_SCENE_GEOMETRY_PATCH_GEOMETRY_H_
