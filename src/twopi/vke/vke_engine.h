#ifndef TWOPI_VK_VK_ENGINE_H_
#define TWOPI_VK_VK_ENGINE_H_

#include <memory>

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

  void Draw();
  void Resize(int width, int height);
  void UpdateCamera(std::shared_ptr<scene::Camera> camera);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_VK_VK_ENGINE_H_
