#include <twopi/scene/camera_control.h>

namespace twopi
{
namespace scene
{
namespace impl
{
class CameraControlImpl
{
public:
  CameraControlImpl()
  {
  }

  ~CameraControlImpl() = default;

  void Update()
  {
  }

  void SetCamera(std::shared_ptr<Camera> camera)
  {
    camera_ = camera;
  }

  std::shared_ptr<Camera> Camera() const
  {
    return camera_;
  }

private:
  std::shared_ptr<scene::Camera> camera_;
};
}

CameraControl::CameraControl()
{
  impl_ = std::make_unique<impl::CameraControlImpl>();
}

CameraControl::~CameraControl() = default;

void CameraControl::Update()
{
  impl_->Update();
}

void CameraControl::SetCamera(std::shared_ptr<scene::Camera> camera)
{
  return impl_->SetCamera(camera);
}

std::shared_ptr<Camera> CameraControl::Camera()
{
  return impl_->Camera();
}
}
}
