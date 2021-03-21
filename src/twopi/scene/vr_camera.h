#ifndef TWOPI_SCENE_VR_CAMERA_H_
#define TWOPI_SCENE_VR_CAMERA_H_

#include <twopi/scene/camera.h>

#include <memory>

namespace twopi
{
namespace scene
{
namespace impl
{
class VrCameraImpl;
}

class VrCamera : public Camera
{
  friend class impl::VrCameraImpl;

public:
  VrCamera();
  ~VrCamera() override;

  glm::mat4 ViewMatrix(int idx) const;
  glm::vec3 Eye(int idx) const;

  float Ipd() const;
  float LensSpacing() const;

private:
  std::unique_ptr<impl::VrCameraImpl> impl_;
};
}
}

#endif // TWOPI_SCENE_VR_CAMERA_H_
