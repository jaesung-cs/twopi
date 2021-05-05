#ifndef TWOPI_VKL_VKL_ENGINE_H_
#define TWOPI_VKL_VKL_ENGINE_H_

#include <memory>
#include <vector>

#include <twopi/core/timestamp.h>

namespace twopi
{
namespace window
{
class Window;
}

namespace scene
{
class Light;
class Camera;
}

namespace vkl
{
class Engine
{
public:
  Engine() = delete;
  explicit Engine(std::shared_ptr<window::Window> window);
  ~Engine();

  void Draw(core::Duration duration);
  void Resize(int width, int height);
  void UpdateLights(const std::vector<std::shared_ptr<scene::Light>>& lights);
  void UpdateCamera(std::shared_ptr<scene::Camera> camera);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_VKL_VKL_ENGINE_H_
