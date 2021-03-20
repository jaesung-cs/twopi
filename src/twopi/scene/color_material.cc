#include <twopi/scene/color_material.h>

#include <glm/glm.hpp>

namespace twopi
{
namespace scene
{
namespace impl
{
class ColorMaterialImpl
{
public:
  ColorMaterialImpl()
  {
  }

  ~ColorMaterialImpl()
  {
  }

  glm::vec3 Color() const
  {
    return color_;
  }

private:
  glm::vec3 color_{ 0.5f, 0.5f, 0.5f };
};
}

ColorMaterial::ColorMaterial()
  : Material()
{
  impl_ = std::make_unique<impl::ColorMaterialImpl>();
}

ColorMaterial::~ColorMaterial() = default;

glm::vec3 ColorMaterial::Color() const
{
  return impl_->Color();
}
}
}
