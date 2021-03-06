#ifndef TWOPI_SCENE_LIGHT_H_
#define TWOPI_SCENE_LIGHT_H_

#include <memory>

#include <glm/fwd.hpp>

namespace twopi
{
namespace scene
{
class Light
{
public:
  Light();
  ~Light();

  void SetDirectionalLight();
  void SetPointLight();
  void SetPosition(const glm::vec3& position);
  void SetAmbient(const glm::vec3& ambient);
  void SetDiffuse(const glm::vec3& diffuse);
  void SetSpecular(const glm::vec3& specular);

  bool IsDirectionalLight() const;
  bool IsPointLight() const;
  const glm::vec3& Position() const;
  const glm::vec3& Ambient() const;
  const glm::vec3& Diffuse() const;
  const glm::vec3& Specular() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_SCENE_LIGHT_H_
