#ifndef TWOPI_SCENE_CAMERA_CONTROL_H_
#define TWOPI_SCENE_CAMERA_CONTROL_H_

#include <memory>

namespace twopi
{
namespace scene
{
class Camera;

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
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_SCENE_CAMERA_CONTROL_H_
