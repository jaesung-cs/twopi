#include <twopi/gpu/gpu_engine.h>

#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <twopi/window/glfw_window.h>
#include <twopi/scene/camera.h>

namespace twopi
{
namespace gpu
{
class Engine::Impl
{
public:
  Impl(std::shared_ptr<window::Window> window)
  {
  }

  ~Impl() = default;

  void Draw()
  {
  }

  void Resize(int width, int height)
  {
  }

  void UpdateCamera(std::shared_ptr<scene::Camera> camera)
  {
  }

private:
  vk::Instance instance_;
  vk::DebugUtilsMessengerEXT messenger_;
};

Engine::Engine(std::shared_ptr<window::Window> window)
  : impl_(std::make_unique<Impl>(window))
{
}

Engine::~Engine()
{
}

void Engine::Draw()
{
  impl_->Draw();
}

void Engine::Resize(int width, int height)
{
  impl_->Resize(width, height);
}

void Engine::UpdateCamera(std::shared_ptr<scene::Camera> camera)
{
  impl_->UpdateCamera(camera);
}
}
}
