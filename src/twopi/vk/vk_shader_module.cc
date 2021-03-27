#include <twopi/vk/vk_shader_module.h>

#include <fstream>

#include <twopi/core/error.h>
#include <twopi/vk/vk_device.h>

namespace twopi
{
namespace vkw
{
//
// Creator
//
ShaderModule::Creator::Creator(Device device)
  : device_(device)
{
}

ShaderModule::Creator::~Creator() = default;

ShaderModule::Creator& ShaderModule::Creator::Load(const std::string& filepath)
{
  std::ifstream file(filepath, std::ios::ate | std::ios::binary);
  if (!file.is_open())
    throw core::Error("Failed to open file: " + filepath);

  size_t file_size = (size_t)file.tellg();
  std::vector<char> buffer(file_size);
  file.seekg(0);
  file.read(buffer.data(), file_size);
  file.close();

  code_.clear();
  auto* int_ptr = reinterpret_cast<uint32_t*>(buffer.data());
  for (int i = 0; i < file_size / 4; i++)
    code_.push_back(int_ptr[i]);

  return *this;
}

ShaderModule ShaderModule::Creator::Create()
{
  create_info_.setCode(code_);

  auto handle = device_.createShaderModule(create_info_);
  return ShaderModule{ device_, handle };
}

//
// ShaderModule
//
ShaderModule::ShaderModule()
{
}

ShaderModule::ShaderModule(vk::Device device, vk::ShaderModule shader_module)
  : device_(device), shader_module_(shader_module)
{
}

ShaderModule::~ShaderModule() = default;

void ShaderModule::Destroy()
{
  device_.destroyShaderModule(shader_module_);
}

ShaderModule::operator vk::ShaderModule() const
{
  return shader_module_;
}
}
}
