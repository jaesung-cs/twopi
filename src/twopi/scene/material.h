#ifndef TWOPI_SCENE_MATERIAL_H_
#define TWOPI_SCENE_MATERIAL_H_

#include <memory>

namespace twopi
{
namespace scene
{
namespace impl
{
class MaterialImpl;
}

class Material
{
public:
  Material();
  virtual ~Material();

private:
  std::unique_ptr<impl::MaterialImpl> impl_;
};
}
}

#endif // TWOPI_SCENE_MATERIAL_H_
