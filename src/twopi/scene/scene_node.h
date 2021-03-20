#ifndef TWOPI_SCENE_SCENE_NODE_H_
#define TWOPI_SCENE_SCENE_NODE_H_

#include <memory>

#include <glm/fwd.hpp>

namespace twopi
{
namespace scene
{
namespace impl
{
class SceneNodeImpl;
}

class SceneNode
{
public:
  SceneNode();
  ~SceneNode();

  const glm::mat4& Transform() const;
  glm::mat4& Transform();

private:
  std::unique_ptr<impl::SceneNodeImpl> impl_;
};
}
}

#endif // TWOPI_SCENE_SCENE_NODE_H_
