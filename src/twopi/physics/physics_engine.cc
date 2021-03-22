#include <twopi/physics/physics_engine.h>

#include <twopi/physics/cloth/mesh_cloth.h>

namespace twopi
{
namespace physics
{
class PhysicsEngine::Impl
{
public:
  Impl()
  {
    cloth_ = std::make_shared<MeshCloth>(100);
  }

  ~Impl() = default;

  void Run()
  {
  }

  void AcquireScene()
  {
  }

private:
  double timestep_ = 1. / 60.;

  std::shared_ptr<MeshCloth> cloth_;
};

PhysicsEngine::PhysicsEngine()
{
  impl_ = std::make_unique<Impl>();
}

PhysicsEngine::~PhysicsEngine() = default;

void PhysicsEngine::Run()
{
  impl_->Run();
}

void PhysicsEngine::AcquireScene()
{
  impl_->AcquireScene();
}
}
}
