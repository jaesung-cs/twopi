#include <twopi/scene/camera_orbit_control.h>

#include <iostream>

#include <twopi/scene/camera.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace twopi
{
namespace scene
{
namespace impl
{
class CameraOrbitControlImpl
{
public:
  CameraOrbitControlImpl() = delete;

  CameraOrbitControlImpl(CameraOrbitControl* base)
    : base_(base)
  {
  }

  ~CameraOrbitControlImpl() = default;

  void Update(std::shared_ptr<Camera> camera)
  {
    const auto cos_theta = std::cos(theta_);
    const auto sin_theta = std::sin(theta_);
    const auto cos_phi = std::cos(phi_);
    const auto sin_phi = std::sin(phi_);

    const glm::vec3 eye = center_ + radius_ * glm::vec3(cos_theta * cos_phi, sin_theta * cos_phi, sin_phi);

    camera->LookAt(eye, center_, up_);
  }

  void TranslateByPixels(int dx, int dy)
  {
    const auto cos_theta = std::cos(theta_);
    const auto sin_theta = std::sin(theta_);
    const auto cos_phi = std::cos(phi_);
    const auto sin_phi = std::sin(phi_);

    const glm::vec3 x = radius_ * glm::vec3(-sin_theta, cos_theta, 0.f);
    const glm::vec3 y = radius_ * glm::vec3(cos_theta * -sin_phi, sin_theta * -sin_phi, cos_phi);

    center_ += translation_sensitivity_ * (-x * static_cast<float>(dx) + y * static_cast<float>(dy));
  }

  void RotateByPixels(int dx, int dy)
  {
    constexpr float epsilon = 1e-6f;
    constexpr auto phi_limit = glm::pi<float>() / 2.f - epsilon;

    theta_ -= rotation_sensitivity_ * dx;
    phi_ = clamp(phi_ + rotation_sensitivity_ * dy, -phi_limit, phi_limit);
  }

  void ZoomByPixels(int dx, int dy)
  {
    radius_ *= std::pow(1.f + zoom_multiplier_, zoom_sensitivity_ * dy);
  }

  void ZoomByWheel(int scroll)
  {
    radius_ /= std::pow(1.f + zoom_multiplier_, zoom_wheel_sensitivity_ * scroll);
  }

private:
  template <typename T>
  T clamp(T value, T min, T max)
  {
    return value < min ? min : value > max ? max : value;
  }

  CameraOrbitControl* base_ = nullptr;

  // pos = center + radius * (cos theta cos phi, sin theta cos phi, sin phi)
  glm::vec3 center_{ 0.f, 0.f, 0.f };
  const glm::vec3 up_{ 0.f, 0.f, 1.f };
  float radius_ = 10.f;
  float theta_ = 0.f;
  float phi_ = 0.f;

  float translation_sensitivity_ = 0.003f;
  float rotation_sensitivity_ = 0.003f;
  float zoom_sensitivity_ = 0.1f;
  float zoom_wheel_sensitivity_ = 5.f;
  float zoom_multiplier_ = 0.01f;
};
}

CameraOrbitControl::CameraOrbitControl()
  : CameraControl()
{
  impl_ = std::make_unique<impl::CameraOrbitControlImpl>(this);
}

CameraOrbitControl::~CameraOrbitControl() = default;

void CameraOrbitControl::Update()
{
  impl_->Update(Camera());
}

void CameraOrbitControl::TranslateByPixels(int dx, int dy)
{
  return impl_->TranslateByPixels(dx, dy);
}

void CameraOrbitControl::RotateByPixels(int dx, int dy)
{
  return impl_->RotateByPixels(dx, dy);
}

void CameraOrbitControl::ZoomByPixels(int dx, int dy)
{
  return impl_->ZoomByPixels(dx, dy);
}

void CameraOrbitControl::ZoomByWheel(int scroll)
{
  return impl_->ZoomByWheel(scroll);
}
}
}
