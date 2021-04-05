#ifndef TWOPI_VKW_VKW_IMAGE_H_
#define TWOPI_VKW_VKW_IMAGE_H_

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Device;
class DeviceMemory;

class Image
{
public:
  class Creator
  {
  public:
    Creator() = delete;
    explicit Creator(Device device);
    ~Creator();

    Creator& SetTransferSrc();
    Creator& SetMipLevels(int mip_levels);
    Creator& SetMultisample4();
    Creator& SetTransientColorAttachment();
    Creator& SetDepthStencilImage();
    Creator& SetFormat(vk::Format format);
    Creator& SetSize(int width, int height);

    Image Create();

  private:
    const vk::Device device_;

    vk::ImageCreateInfo create_info_{};
    vk::Format format_{};
    int width_;
    int height_;
  };

public:
  Image();
  Image(vk::Image image);
  Image(vk::Image image, vk::Format format);
  Image(vk::Device device, vk::Image image);
  Image(vk::Device device, vk::Image image, vk::Format format);

  ~Image();

  void Destroy();

  operator vk::Image() const;

  vk::Format Format() const;

  void Bind(DeviceMemory memory, uint64_t offset = 0);

  auto Width() const { return width_; }
  auto Height() const { return height_; }

private:
  vk::Device device_;
  vk::Image image_;
  vk::Format format_;
  int width_;
  int height_;
};
}
}

#endif // TWOPI_VKW_VKW_IMAGE_H_
