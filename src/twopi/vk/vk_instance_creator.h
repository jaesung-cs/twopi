#ifndef TWOPI_VK_VK_INSTANCE_CREATOR_H_
#define TWOPI_VK_VK_INSTANCE_CREATOR_H_

#include <memory>

namespace twopi
{
namespace vk
{
class Instance;

class InstanceCreator
{
public:
  InstanceCreator();
  ~InstanceCreator();

  InstanceCreator& AddGlfwRequiredExtensions();

  Instance Create();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_VK_VK_INSTANCE_CREATOR_H_
