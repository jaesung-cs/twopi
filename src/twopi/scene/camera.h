#ifndef TWOPI_SCENE_CAMERA_H_
#define TWOPI_SCENE_CAMERA_H_

#include <memory>

#include <glm/fwd.hpp>

namespace twopi
{
namespace scene
{
namespace impl
{
class CameraImpl;
}

class Camera
{
public:
  Camera();
  ~Camera();

  void SetPerspective();
  void SetOrtho();

  void SetFovy(float fovy);
  void SetZoom(float zoom);
  void SetNearFar(float near, float far);
  void SetScreenSize(int width, int height);

  void LookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up);

  glm::mat4 ProjectionMatrix() const;
  glm::mat4 ViewMatrix() const;

private:
  std::unique_ptr<impl::CameraImpl> impl_;
};
}
}

#endif // TWOPI_SCENE_CAMERA_H_
