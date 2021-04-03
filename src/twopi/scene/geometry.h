#ifndef TWOPI_SCENE_GEOMETRY_H_
#define TWOPI_SCENE_GEOMETRY_H_

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
  ~Geometry();

  void AddAttribute(std::vector<float>&& attribute);
  void AddAttribute(const std::vector<float>& attribute);

  const std::vector<float>& Attribute(int index) const;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_SCENE_GEOMETRY_H_
