#ifndef TWOPI_VKE_VKE_ENGINE_H_
#define TWOPI_VKE_VKE_ENGINE_H_

#include <memory>

#include <twopi/core/timestamp.h>

namespace twopi
{
namespace window
{
class Window;
}

namespace scene
{
class Camera;
}

namespace vke
{
class Engine
{
public:
  Engine() = delete;
  explicit Engine(std::shared_ptr<window::Window> window);
  ~Engine();

  void Draw(core::Duration duration);
  void Resize(int width, int height);
  void UpdateCamera(std::shared_ptr<scene::Camera> camera);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_VKE_VKE_ENGINE_H_
