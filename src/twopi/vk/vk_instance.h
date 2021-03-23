#ifndef TWOPI_VK_VK_INSTANCE_H_
#define TWOPI_VK_VK_INSTANCE_H_

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

namespace twopi
{
namespace vk
{
class Instance
{
public:
  static std::vector<VkExtensionProperties> Extensions();

public:
  Instance();
  Instance(VkInstance instance);

  Instance(const Instance& rhs);
  Instance& operator = (const Instance& rhs);

  Instance(Instance&& rhs);
  Instance& operator = (Instance&& rhs);

  ~Instance();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_VK_VK_INSTANCE_H_
