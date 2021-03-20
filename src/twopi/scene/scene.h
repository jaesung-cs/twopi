#ifndef TWOPI_SCENE_SCENE_H_
#define TWOPI_SCENE_SCENE_H_

#include <memory>

namespace twopi
{
namespace scene
{
class SceneNode;

namespace impl
{
class SceneImpl;
}

class Scene
{
public:
  Scene();
  ~Scene();

  std::shared_ptr<SceneNode> Root() const;

private:
  std::unique_ptr<impl::SceneImpl> impl_;
};
}
}

#endif // TWOPI_SCENE_SCENE_H_
