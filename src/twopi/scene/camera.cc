#include <twopi/scene/camera.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace twopi
{
namespace scene
{
class Camera::Impl
{
private:
  enum class Type
  {
    PERSPECTIVE,
    ORTHO,
  };

public:
  Impl()
  {
  }

  ~Impl() = default;

  void SetPerspective()
  {
    type_ = Type::PERSPECTIVE;
  }

  void SetOrtho()
  {
    type_ = Type::ORTHO;
  }

  void SetFovy(float fovy)
  {
    fovy_ = fovy;
  }

  void SetZoom(float zoom)
  {
    zoom_ = zoom;
  }

  void SetNearFar(float near, float far)
  {
    near_ = near;
    far_ = far;
  }

  void SetScreenSize(int width, int height)
  {
    width_ = width;
    height_ = height;
  }

  void LookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up)
  {
    eye_ = eye;
    center_ = center;
    up_ = up;
  }

  glm::mat4 ProjectionMatrix() const
  {
    const auto aspect = static_cast<float>(width_) / height_;

    switch (type_)
    {
    case Type::PERSPECTIVE:
      return glm::perspective(glm::degrees(fovy_), aspect, near_, far_);
    case Type::ORTHO:
      return glm::ortho(-aspect * zoom_, aspect * zoom_, -zoom_, zoom_, near_, far_);
    }
  }

  glm::mat4 ViewMatrix() const
  {
    return glm::lookAt(eye_, center_, up_);
  }

  const auto& Eye() const { return eye_; }
  const auto& Center() const { return center_; }
  const auto& Up() const { return up_; }

private:
  Type type_ = Type::PERSPECTIVE;

  int width_ = 1;
  int height_ = 1;

  float near_ = 0.01f;
  float far_ = 1000.f;

  float fovy_ = 45.f / 180.f * glm::pi<float>();

  float zoom_ = 1.f;

  glm::vec3 eye_;
  glm::vec3 center_;
  glm::vec3 up_;
};

Camera::Camera()
{
  impl_ = std::make_unique<Impl>();
}

Camera::~Camera() = default;

void Camera::SetPerspective()
{
  impl_->SetPerspective();
}

void Camera::SetOrtho()
{
  impl_->SetOrtho();
}

void Camera::SetFovy(float fovy)
{
  impl_->SetFovy(fovy);
}

void Camera::SetZoom(float zoom)
{
  impl_->SetZoom(zoom);
}

void Camera::SetNearFar(float near, float far)
{
  impl_->SetNearFar(near, far);
}

void Camera::SetScreenSize(int width, int height)
{
  impl_->SetScreenSize(width, height);
}

void Camera::LookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up)
{
  impl_->LookAt(eye, center, up);
}

glm::mat4 Camera::ProjectionMatrix() const
{
  return impl_->ProjectionMatrix();
}

glm::mat4 Camera::ViewMatrix() const
{
  return impl_->ViewMatrix();
}

const glm::vec3& Camera::Eye() const
{
  return impl_->Eye();
}

const glm::vec3& Camera::Center() const
{
  return impl_->Center();
}

const glm::vec3& Camera::Up() const
{
  return impl_->Up();
}
}
}
