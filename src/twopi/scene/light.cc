#include <twopi/scene/light.h>

#include <glm/glm.hpp>

namespace twopi
{
namespace scene
{
class Light::Impl
{
private:
  enum class Type
  {
    DIRECTIONAL,
    POINT,
  };

public:
  Impl()
  {
  }

  ~Impl() = default;

  void SetDirectionalLight() { type_ = Type::DIRECTIONAL; }
  void SetPointLight() { type_ = Type::POINT; }
  void SetPosition(const glm::vec3& position) { position_ = position; }
  void SetAmbient(const glm::vec3& ambient) { ambient_ = ambient; }
  void SetDiffuse(const glm::vec3& diffuse) { diffuse_ = diffuse; }
  void SetSpecular(const glm::vec3& specular) { specular_ = specular; }

  bool IsDirectionalLight() const { return type_ == Type::DIRECTIONAL; }
  bool IsPointLight() const { return type_ == Type::POINT; }
  const glm::vec3& Position() const { return position_; }
  const glm::vec3& Ambient() const { return ambient_; }
  const glm::vec3& Diffuse() const { return diffuse_; }
  const glm::vec3& Specular() const { return specular_; }

private:
  Type type_;
  glm::vec3 position_{ 0.f, 0.f, 1.f };
  glm::vec3 ambient_{ 0.f, 0.f, 0.f };
  glm::vec3 diffuse_{ 0.f, 0.f, 0.f };
  glm::vec3 specular_{ 0.f, 0.f, 0.f };
};

Light::Light()
{
  impl_ = std::make_unique<Impl>();
}

Light::~Light() = default;

void Light::SetDirectionalLight()
{
  impl_->SetDirectionalLight();
}

void Light::SetPointLight()
{
  impl_->SetPointLight();
}

void Light::SetPosition(const glm::vec3& position)
{
  impl_->SetPosition(position);
}

void Light::SetAmbient(const glm::vec3& ambient)
{
  impl_->SetAmbient(ambient);
}

void Light::SetDiffuse(const glm::vec3& diffuse)
{
  impl_->SetDiffuse(diffuse);
}

void Light::SetSpecular(const glm::vec3& specular)
{
  impl_->SetSpecular(specular);
}

bool Light::IsDirectionalLight() const
{
  return impl_->IsDirectionalLight();
}

bool Light::IsPointLight() const
{
  return impl_->IsPointLight();
}

const glm::vec3& Light::Position() const
{
  return impl_->Position();
}

const glm::vec3& Light::Ambient() const
{
  return impl_->Ambient();
}

const glm::vec3& Light::Diffuse() const
{
  return impl_->Diffuse();
}

const glm::vec3& Light::Specular() const
{
  return impl_->Specular();
}
}
}
