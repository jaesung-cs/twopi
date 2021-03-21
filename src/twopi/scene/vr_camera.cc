#include <twopi/scene/vr_camera.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace twopi
{
namespace scene
{
namespace impl
{
class VrCameraImpl
{
public:
  VrCameraImpl() = delete;

  VrCameraImpl(VrCamera* base)
    : base_(base)
  {
  }

  ~VrCameraImpl() = default;

  glm::mat4 ViewMatrix(int idx) const
  {
    auto lateral_offset = (ipd_ / 2.f) * (idx == 0 ? -1 : 1);

    const auto& eye = static_cast<Camera*>(base_)->Eye();
    const auto direction = glm::normalize(base_->Center() - eye);
    const auto up = glm::normalize(base_->Up());
    const auto lateral = glm::cross(direction, up);

    return glm::lookAt(eye + lateral * lateral_offset, eye + direction * lens_spacing_, up);
  }

  glm::vec3 Eye(int idx) const
  {
    auto lateral_offset = (ipd_ / 2.f) * (idx == 0 ? -1 : 1);

    const auto& eye = static_cast<Camera*>(base_)->Eye();
    const auto direction = glm::normalize(base_->Center() - eye);
    const auto up = glm::normalize(base_->Up());
    const auto lateral = glm::cross(direction, up);

    return eye + lateral * lateral_offset;
  }

  float Ipd() const
  {
    return ipd_;
  }

  float LensSpacing() const
  {
    return lens_spacing_;
  }

private:
  VrCamera* base_;

  // IPD default based on Oculus Quest 2, https://support.oculus.com/351344152731317/
  float ipd_ = 0.063; // default is 63mm, by Oculus Quest 2
  float lens_spacing_ = 1.f;
};
}

VrCamera::VrCamera()
  : Camera()
{
  impl_ = std::make_unique<impl::VrCameraImpl>(this);
}

VrCamera::~VrCamera() = default;

glm::mat4 VrCamera::ViewMatrix(int idx) const
{
  return impl_->ViewMatrix(idx);
}

glm::vec3 VrCamera::Eye(int idx) const
{
  return impl_->Eye(idx);
}

float VrCamera::Ipd() const
{
  return impl_->Ipd();
}

float VrCamera::LensSpacing() const
{
  return impl_->LensSpacing();
}
}
}
