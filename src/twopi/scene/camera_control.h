#ifndef TWOPI_SCENE_CAMERA_CONTROL_H_
#define TWOPI_SCENE_CAMERA_CONTROL_H_

#include <memory>

namespace twopi
{
namespace scene
{
class Camera;

namespace impl
{
class CameraControlImpl;
}

class CameraControl
{
public:
  CameraControl();
  virtual ~CameraControl();

  virtual void Update();

  void SetCamera(std::shared_ptr<Camera> camera);

protected:
  std::shared_ptr<Camera> Camera();

private:
  std::unique_ptr<impl::CameraControlImpl> impl_;
};
}
}

#endif // TWOPI_SCENE_CAMERA_CONTROL_H_
