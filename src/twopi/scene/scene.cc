#include <twopi/scene/scene.h>

#include <twopi/scene/scene_node.h>

namespace twopi
{
namespace scene
{
class Scene::Impl
{
public:
  Impl()
  {
    root_ = std::make_shared<SceneNode>();
  }

  ~Impl() = default;

  std::shared_ptr<SceneNode> Root() const
  {
    return root_;
  }

private:
  std::shared_ptr<SceneNode> root_;
};

Scene::Scene()
{
  impl_ = std::make_unique<Impl>();
}

Scene::~Scene() = default;

std::shared_ptr<SceneNode> Scene::Root() const
{
  return impl_->Root();
}
}
}
