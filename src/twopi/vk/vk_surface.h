#ifndef TWOPI_VK_VK_SURFACE_H_
#define TWOPI_VK_VK_SURFACE_H_

#include <vulkan/vulkan.hpp>

struct GLFWwindow;

namespace twopi
{
namespace vkw
{
class Instance;

class Surface
{
public:
  class Creator
  {
  public:
    Creator() = delete;
    Creator(Instance instance, GLFWwindow* window);
    ~Creator();

    Surface Create();

  private:
    vk::Instance instance_;
    GLFWwindow* window_;
  };

public:
  Surface();
  Surface(vk::Instance instance, vk::SurfaceKHR surface);

  ~Surface();
  
  void Destroy();

  operator vk::SurfaceKHR() const;

private:
  vk::Instance instance_;
  vk::SurfaceKHR surface_;
};
}
}

#endif // TWOPI_VK_VK_SURFACE_H_
