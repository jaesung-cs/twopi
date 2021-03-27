#include <twopi/vk/vk_surface.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <twopi/vk/vk_instance.h>

namespace twopi
{
namespace vkw
{
//
// Creator
//
Surface::Creator::Creator(Instance instance, GLFWwindow* window)
  : instance_(instance), window_(window)
{
}

Surface::Creator::~Creator() = default;

Surface Surface::Creator::Create()
{
  VkSurfaceKHR surface;
  glfwCreateWindowSurface(instance_, window_, nullptr, &surface);
  return Surface{ instance_, surface };
}

//
// Surface
//
Surface::Surface()
{
}

Surface::Surface(vk::Instance instance, vk::SurfaceKHR surface)
  : instance_(instance_), surface_(surface)
{
}

Surface::~Surface() = default;

void Surface::Destroy()
{
  instance_.destroy(surface_);
}

Surface::operator vk::SurfaceKHR() const
{
  return surface_;
}
}
}
