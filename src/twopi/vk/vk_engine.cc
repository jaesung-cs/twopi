#include <twopi/vk/vk_engine.h>

#include <iostream>

#include <twopi/vk/vk_instance_creator.h>
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

    instance_ = InstanceCreator{}
      .AddGlfwRequiredExtensions()
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
