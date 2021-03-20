#include <twopi/scene/material.h>

namespace twopi
{
namespace scene
{
namespace impl
{
class MaterialImpl
{
public:
  MaterialImpl()
  {
  }

  ~MaterialImpl()
  {
  }

private:
};
}

Material::Material()
{
  impl_ = std::make_unique<impl::MaterialImpl>();
}

Material::~Material() = default;
}
}
