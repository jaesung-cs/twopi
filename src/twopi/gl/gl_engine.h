#ifndef TWOPI_GL_GL_ENGINE_H_
#define TWOPI_GL_GL_ENGINE_H_

#include <memory>

namespace twopi
{
namespace scene
{
class Camera;
}

namespace gl
{
namespace impl
{
class EngineImpl;
}

class Engine
{
public:
  Engine();
  ~Engine();

  void SetViewport(int x, int y, int width, int height);
  void Draw();

  void UpdateCamera(std::shared_ptr<scene::Camera> camera);

private:
  std::unique_ptr<impl::EngineImpl> impl_;
};
}
}

#endif // TWOPI_GL_GL_ENGINE_H_
