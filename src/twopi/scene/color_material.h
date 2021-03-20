#ifndef TWOPI_SCENE_COLOR_MATERIAL_H_
#define TWOPI_SCENE_COLOR_MATERIAL_H_

#include <twopi/scene/material.h>

#include <memory>

#include <glm/fwd.hpp>

namespace twopi
{
namespace scene
{
namespace impl
{
class ColorMaterialImpl;
}

class ColorMaterial : public Material
{
public:
  ColorMaterial();
  ~ColorMaterial() override;

  glm::vec3 Color() const;

private:
  std::unique_ptr<impl::ColorMaterialImpl> impl_;
};
}
}

#endif // TWOPI_SCENE_COLOR_MATERIAL_H_
