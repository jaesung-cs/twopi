#include <twopi/vk/vk_engine.h>

#include <iostream>

#include <twopi/vk/vk_instance.h>

namespace twopi
{
namespace vk
{
class Engine::Impl
{
public:
  Impl()
  {
    const auto extensions = Instance::Extensions();
    std::cout << "Available instance extensions:" << std::endl;
    for (const auto& extension : extensions)
      std::cout << "  " << extension.extensionName << std::endl;
    std::cout << std::endl;

    const auto layers = Instance::Layers();
    std::cout << "Available instance layers:" << std::endl;
    for (const auto& layer : layers)
      std::cout << "  " << layer.layerName << ": " << layer.description << std::endl;
    std::cout << std::endl;

    instance_ = Instance::Creator{}
      .AddGlfwRequiredExtensions()
      .EnableValidationLayer()
      .Create();
  }

  ~Impl()
  {
  }

private:
  vk::Instance instance_;
};

Engine::Engine()
{
  impl_ = std::make_unique<Impl>();
}

Engine::~Engine() = default;
}
}
