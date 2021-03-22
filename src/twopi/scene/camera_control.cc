#include <twopi/scene/camera_control.h>

namespace twopi
{
namespace scene
{
class CameraControl::Impl
{
public:
  Impl()
  {
  }

  ~Impl() = default;

  void Update()
  {
  }

  void SetCamera(std::shared_ptr<scene::Camera> camera)
  {
    camera_ = camera;
  }

  std::shared_ptr<scene::Camera> Camera() const
  {
    return camera_;
  }

private:
  std::shared_ptr<scene::Camera> camera_;
};

CameraControl::CameraControl()
{
  impl_ = std::make_unique<Impl>();
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
