#include <twopi/vk/vk_engine.h>

#include <iostream>

#include <twopi/vk/vk_instance.h>
#include <twopi/vk/vk_physical_device.h>
#include <twopi/vk/vk_device.h>
#include <twopi/vk/vk_queue.h>
#include <twopi/vk/vk_surface.h>

namespace twopi
{
namespace vkw
{
class Engine::Impl
{
public:
  Impl() = delete;

  Impl(GLFWwindow* window)
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
      if (physical_device.Properties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
        std::cout << "    " << "Discrete GPU" << std::endl;
      if (physical_device.Features().geometryShader)
        std::cout << "    " << "Has Geometry Shader" << std::endl;
    }

    surface_ = Surface::Creator{ instance_, window }.Create();

    // TODO: pick the most suitable device, now simply use physical device of index 0
    // TODO: setup queue settings
    device_ = Device::Creator{ physical_devices[0] }
      .AddGraphicsQueue()
      .AddPresentQueue(surface_)
      .Create();

    graphics_queue_ = device_.Queue(0);
    present_queue_ = device_.Queue(1);
  }
  
  ~Impl()
  {
    surface_.Destroy();
    device_.Destroy();
    instance_.Destroy();
  }

  void Draw()
  {
    // TODO
  }

private:
  vkw::Instance instance_;
  vkw::Device device_;
  vkw::Queue graphics_queue_;
  vkw::Queue present_queue_;
  vkw::Surface surface_;
};

Engine::Engine(GLFWwindow* window)
{
  impl_ = std::make_unique<Impl>(window);
}

Engine::~Engine() = default;

void Engine::Draw()
{
  impl_->Draw();
}
}
}
