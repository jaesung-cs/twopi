#ifndef TWOPI_VK_VK_BUFFER_H_
#define TWOPI_VK_VK_BUFFER_H_

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Device;
class DeviceMemory;

class Buffer
{
public:
  class Creator
  {
  public:
    Creator() = delete;
    explicit Creator(Device device);
    ~Creator();

    Creator& SetVertexBufferSize(int size);

    Buffer Create();

  private:
    const vk::Device device_;

    vk::BufferCreateInfo create_info_{};
  };

public:
  Buffer();
  Buffer(vk::Device device, vk::Buffer buffer);

  ~Buffer();

  void Destroy();

  operator vk::Buffer() const;

  void Bind(DeviceMemory memory);

private:
  vk::Device device_;
  vk::Buffer buffer_;
};
}
}

#endif // TWOPI_VK_VK_BUFFER_H_
