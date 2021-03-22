#ifndef TWOPI_SCENE_SCENE_H_
#define TWOPI_SCENE_SCENE_H_

#include <memory>

namespace twopi
{
namespace scene
{
class SceneNode;

class Scene
{
public:
  Scene();
  ~Scene();

  std::shared_ptr<SceneNode> Root() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_SCENE_SCENE_H_
