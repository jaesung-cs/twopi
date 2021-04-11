#include <twopi/vkl/vkl_engine.h>

#include <iostream>

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkl
{
class Engine::Impl
{
public:
  Impl() = delete;

  Impl(std::shared_ptr<window::Window> window)
  {
    Prepare();
  }

  void Draw(core::Duration duration)
  {
    std::cout << "Draw " << duration.count() << std::endl;
  }

  void Resize(int width, int height)
  {
    std::cout << "Resized: " << width << ' ' << height << std::endl;
  }

  void UpdateLights(const std::vector<std::shared_ptr<scene::Light>>& lights)
  {
  }

  void UpdateCamera(std::shared_ptr<scene::Camera> camera)
  {
  }

private:
  void Prepare()
  {
    CreateInstance();
  }

  void CreateInstance()
  {
  }

  vk::Instance instance_;
};

Engine::Engine(std::shared_ptr<window::Window> window)
  : impl_(std::make_unique<Impl>(window))
{
}

Engine::~Engine() = default;

void Engine::Draw(core::Duration duration)
{
  impl_->Draw(duration);
}

void Engine::Resize(int width, int height)
{
  impl_->Resize(width, height);
}

void Engine::UpdateLights(const std::vector<std::shared_ptr<scene::Light>>& lights)
{
  impl_->UpdateLights(lights);
}

void Engine::UpdateCamera(std::shared_ptr<scene::Camera> camera)
{
  impl_->UpdateCamera(camera);
}
}
}
