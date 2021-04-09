#ifndef TWOPI_VKW_VKW_BUFFER_H_
#define TWOPI_VKW_VKW_BUFFER_H_

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

    Creator& SetSize(uint64_t size);
    Creator& SetTransferSrcBuffer();
    Creator& SetTransferDstBuffer();
    Creator& SetVertexBuffer();
    Creator& SetIndexBuffer();
    Creator& SetUniformBuffer();

    Buffer Create();

  private:
    const vk::Device device_;

    vk::BufferCreateInfo create_info_{};
    vk::BufferUsageFlags usage_{};
  };

public:
  Buffer();
  Buffer(vk::Device device, vk::Buffer buffer);

  ~Buffer();

  void Destroy();

  operator vk::Buffer() const;

  uint64_t Size() const;

  void Bind(DeviceMemory memory, uint64_t offset = 0);

private:
  vk::Device device_;
  vk::Buffer buffer_;

  uint64_t size_;
};
}
}

#endif // TWOPI_VKW_VKW_BUFFER_H_
