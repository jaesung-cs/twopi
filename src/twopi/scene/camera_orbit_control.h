#ifndef TWOPI_SCENE_CAMERA_ORBIT_CONTROL_H_
#define TWOPI_SCENE_CAMERA_ORBIT_CONTROL_H_

#include <twopi/scene/camera_control.h>

#include <memory>

namespace twopi
{
namespace scene
{
class CameraOrbitControl : public CameraControl
{
  friend class CameraOrbitControlImpl;

public:
  CameraOrbitControl();
  ~CameraOrbitControl() override;

  void Update() override;

  void TranslateByPixels(int dx, int dy);
  void RotateByPixels(int dx, int dy);
  void ZoomByPixels(int dx, int dy);
  void ZoomByWheel(int scroll);

  void MoveForward(float dt);
  void MoveRight(float dt);
  void MoveUp(float dt);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_SCENE_CAMERA_ORBIT_CONTROL_H_
