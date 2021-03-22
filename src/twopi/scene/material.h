#ifndef TWOPI_SCENE_MATERIAL_H_
#define TWOPI_SCENE_MATERIAL_H_

#include <memory>

namespace twopi
{
namespace scene
{
class Material
{
public:
  Material();
  virtual ~Material();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_SCENE_MATERIAL_H_
