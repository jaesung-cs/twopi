#ifndef TWOPI_VK_VK_IMAGE_VIEW_H_
#define TWOPI_VK_VK_IMAGE_VIEW_H_

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Device;
class Image;

class ImageView
{
public:
  class Creator
  {
  public:
    Creator() = delete;
    Creator(Device device);
    ~Creator();

    Creator& SetImage(Image image);

    ImageView Create();

  private:
    vk::Device device_;

    vk::ImageViewCreateInfo create_info_{};
  };

public:
  ImageView();
  ImageView(vk::Device device_, vk::ImageView image_view);

  ~ImageView();

  operator vk::ImageView() const;

  void Destroy();

private:
  vk::Device device_;
  vk::ImageView image_view_;
};
}
}

#endif // TWOPI_VK_VK_IMAGE_VIEW_H_
