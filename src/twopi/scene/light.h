#ifndef TWOPI_SCENE_LIGHT_H_
#define TWOPI_SCENE_LIGHT_H_

#include <memory>

#include <glm/fwd.hpp>

namespace twopi
{
namespace scene
{
namespace impl
{
class LightImpl;
}

class Light
{
public:
  Light();
  ~Light();

  void SetPosition(const glm::vec3& position);
  void SetAmbient(const glm::vec3& ambient);
  void SetDiffuse(const glm::vec3& diffuse);
  void SetSpecular(const glm::vec3& specular);

  const glm::vec3& Position() const;
  const glm::vec3& Ambient() const;
  const glm::vec3& Diffuse() const;
  const glm::vec3& Specular() const;

private:
  std::unique_ptr<impl::LightImpl> impl_;
};
}
}

#endif // TWOPI_SCENE_LIGHT_H_
