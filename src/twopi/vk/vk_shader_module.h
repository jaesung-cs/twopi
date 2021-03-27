#ifndef TWOPI_VK_VK_SHADER_MODULE_H_
#define TWOPI_VK_VK_SHADER_MODULE_H_

#include <string>
#include <vector>

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Device;

class ShaderModule
{
public:
  class Creator
  {
  public:
    Creator() = delete;
    explicit Creator(Device device);
    ~Creator();

    Creator& Load(const std::string& filepath);

    ShaderModule Create();

  private:
    const vk::Device device_;

    vk::ShaderModuleCreateInfo create_info_{};

    std::vector<uint32_t> code_;
  };

public:
  ShaderModule();
  ShaderModule(vk::Device device, vk::ShaderModule shader_module);

  ~ShaderModule();

  void Destroy();

  operator vk::ShaderModule() const;

private:
  vk::Device device_;
  vk::ShaderModule shader_module_;
};
}
}

#endif // TWOPI_VK_VK_SHADER_MODULE_H_
