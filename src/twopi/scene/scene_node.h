#ifndef TWOPI_SCENE_SCENE_NODE_H_
#define TWOPI_SCENE_SCENE_NODE_H_

#include <memory>

#include <glm/fwd.hpp>

namespace twopi
{
namespace scene
{
class SceneNode
{
public:
  SceneNode();
  ~SceneNode();

  const glm::mat4& Transform() const;
  glm::mat4& Transform();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_SCENE_SCENE_NODE_H_
