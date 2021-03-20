#ifndef TWOPI_SCENE_CAMERA_ORBIT_CONTROL_H_
#define TWOPI_SCENE_CAMERA_ORBIT_CONTROL_H_

#include <twopi/scene/camera_control.h>

#include <memory>

namespace twopi
{
namespace scene
{
namespace impl
{
class CameraOrbitControlImpl;
}

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

private:
  std::unique_ptr<impl::CameraOrbitControlImpl> impl_;
};
}
}

#endif // TWOPI_SCENE_CAMERA_ORBIT_CONTROL_H_
