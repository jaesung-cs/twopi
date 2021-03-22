#include <twopi/scene/material.h>

namespace twopi
{
namespace scene
{
class Material::Impl
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

Material::Material()
{
  impl_ = std::make_unique<Impl>();
}

Material::~Material() = default;
}
}
