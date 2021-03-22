#ifndef TWOPI_PHYSICS_PHYSICS_ENGINE_H_
#define TWOPI_PHYSICS_PHYSICS_ENGINE_H_

#include <memory>

namespace twopi
{
namespace physics
{
class PhysicsEngine
{
public:
  PhysicsEngine();
  ~PhysicsEngine();

  void Run();

  void AcquireScene();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_PHYSICS_PHYSICS_ENGINE_H_
