#ifndef TWOPI_GL_PHYSICS_GL_PHYSICS_SIMULATOR_H_
#define TWOPI_GL_PHYSICS_GL_PHYSICS_SIMULATOR_H_

#include <memory>

namespace twopi
{
namespace gl
{
namespace physics
{
class PhysicsSimulator
{
public:
  PhysicsSimulator();
  ~PhysicsSimulator();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}
}

#endif // TWOPI_GL_PHYSICS_GL_PHYSICS_SIMULATOR_H_
