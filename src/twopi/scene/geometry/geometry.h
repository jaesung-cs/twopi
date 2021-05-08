#ifndef TWOPI_SCENE_GEOMETRY_GEOMETRY_H_
#define TWOPI_SCENE_GEOMETRY_GEOMETRY_H_

#include <memory>
#include <vector>

namespace twopi
{
namespace scene
{
class Geometry
{
public:
  Geometry();
  virtual ~Geometry();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_SCENE_GEOMETRY_GEOMETRY_H_
