#include <twopi/vk/vk_engine.h>

#include <iostream>

#include <twopi/vk/vk_instance.h>
#include <twopi/vk/vk_physical_device.h>

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

    std::cout << "Physical devices:" << std::endl;
    const auto physical_devices = instance_.PhysicalDevices();
    for (const auto& physical_device : physical_devices)
    {
      std::cout << "  " << physical_device.Properties().deviceName << std::endl;
      if (physical_device.Properties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        std::cout << "    " << "Discrete GPU" << std::endl;
      if (physical_device.Features().geometryShader)
        std::cout << "    " << "Has Geometry Shader" << std::endl;
    }
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
