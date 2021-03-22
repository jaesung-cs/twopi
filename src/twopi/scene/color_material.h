#ifndef TWOPI_SCENE_COLOR_MATERIAL_H_
#define TWOPI_SCENE_COLOR_MATERIAL_H_

#include <twopi/scene/material.h>

#include <memory>

#include <glm/fwd.hpp>

namespace twopi
{
namespace scene
{
class ColorMaterial : public Material
{
public:
  ColorMaterial();
  ~ColorMaterial() override;

  glm::vec3 Color() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_SCENE_COLOR_MATERIAL_H_
