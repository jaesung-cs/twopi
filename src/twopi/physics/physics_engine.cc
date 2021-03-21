#include <twopi/physics/physics_engine.h>

#include <twopi/physics/cloth/mesh_cloth.h>

namespace twopi
{
namespace physics
{
namespace impl
{
class PhysicsEngineImpl
{
public:
  PhysicsEngineImpl()
  {
    cloth_ = std::make_shared<MeshCloth>(100);
  }

  ~PhysicsEngineImpl() = default;

private:
  std::shared_ptr<MeshCloth> cloth_;
};
}

PhysicsEngine::PhysicsEngine()
{
  impl_ = std::make_unique<impl::PhysicsEngineImpl>();
}

PhysicsEngine::~PhysicsEngine() = default;
}
}
