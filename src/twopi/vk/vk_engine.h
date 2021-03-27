#ifndef TWOPI_VK_VK_ENGINE_H_
#define TWOPI_VK_VK_ENGINE_H_

#include <memory>

namespace twopi
{
namespace window
{
class Window;
}

namespace vkw
{
class Engine
{
public:
  Engine() = delete;
  explicit Engine(std::shared_ptr<window::Window> window);
  ~Engine();

  void Draw();
  void Resize(int width, int height);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_VK_VK_ENGINE_H_
