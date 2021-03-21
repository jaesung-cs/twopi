#ifndef TWOPI_PHYSICS_PHYSICS_ENGINE_H_
#define TWOPI_PHYSICS_PHYSICS_ENGINE_H_

#include <memory>

namespace twopi
{
namespace physics
{
namespace impl
{
class PhysicsEngineImpl;
}

class PhysicsEngine
{
public:
  PhysicsEngine();
  ~PhysicsEngine();

private:
  std::unique_ptr<impl::PhysicsEngineImpl> impl_;
};
}
}

#endif // TWOPI_PHYSICS_PHYSICS_ENGINE_H_
