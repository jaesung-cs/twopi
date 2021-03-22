#ifndef TWOPI_GL_GL_ENGINE_H_
#define TWOPI_GL_GL_ENGINE_H_

#include <memory>
#include <vector>

namespace twopi
{
namespace scene
{
class Camera;
class Light;
}

namespace gl
{
class Engine
{
public:
  Engine();
  ~Engine();

  void SetViewport(int x, int y, int width, int height);
  void Draw();

  void UpdateLights(const std::vector<std::shared_ptr<scene::Light>>& lights);
  void UpdateCamera(std::shared_ptr<scene::Camera> camera);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_GL_GL_ENGINE_H_
