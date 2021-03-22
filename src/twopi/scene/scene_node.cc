#include <twopi/scene/scene_node.h>

#include <glm/glm.hpp>

namespace twopi
{
namespace scene
{
class SceneNode::Impl
{
public:
  Impl()
  {
  }

  ~Impl() = default;

  const glm::mat4& Transform() const
  {
    return transform_;
  }

  glm::mat4& Transform()
  {
    return transform_;
  }

private:
  glm::mat4 transform_{ 1.f };
};

SceneNode::SceneNode()
{
  impl_ = std::make_unique<Impl>();
}

SceneNode::~SceneNode() = default;

const glm::mat4& SceneNode::Transform() const
{
  return impl_->Transform();
}

glm::mat4& SceneNode::Transform()
{
  return impl_->Transform();
}
}
}
