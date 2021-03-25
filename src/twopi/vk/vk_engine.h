#ifndef TWOPI_VK_VK_ENGINE_H_
#define TWOPI_VK_VK_ENGINE_H_

#include <memory>

namespace twopi
{
namespace vkw
{
class Engine
{
public:
  Engine();
  ~Engine();

  void Draw();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_VK_VK_ENGINE_H_
