#include <twopi/gl/physics/gl_physics_simulator.h>

#include <twopi/gl/physics/gl_mesh_cloth.h>

namespace twopi
{
namespace gl
{
namespace physics
{
class PhysicsSimulator::Impl
{
public:
  Impl()
  {
    cloth_ = std::make_shared<MeshCloth>(100);
  }

  ~Impl() = default;

private:
  double timestep_ = 1. / 60.;

  std::shared_ptr<MeshCloth> cloth_;
};

PhysicsSimulator::PhysicsSimulator()
{
  impl_ = std::make_unique<Impl>();
}

PhysicsSimulator::~PhysicsSimulator() = default;
}
}
}
