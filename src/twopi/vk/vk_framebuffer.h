#ifndef TWOPI_VK_VK_FRAMEBUFFER_H_
#define TWOPI_VK_VK_FRAMEBUFFER_H_

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Device;
class RenderPass;
class ImageView;

class Framebuffer
{
public:
  class Creator
  {
  public:
    Creator() = delete;
    explicit Creator(Device device);
    ~Creator();

    Creator& SetAttachment(ImageView image_view);
    Creator& SetExtent(int width, int height);
    Creator& SetRenderPass(RenderPass render_pass);

    Framebuffer Create();

  private:
    const vk::Device device_;

    vk::FramebufferCreateInfo create_info_{};
  };

public:
  Framebuffer();
  Framebuffer(vk::Device device, vk::Framebuffer framebuffer);

  ~Framebuffer();

  void Destroy();

  operator vk::Framebuffer() const;

private:
  vk::Device device_;
  vk::Framebuffer framebuffer_;
};
}
}

#endif // TWOPI_VK_VK_FRAMEBUFFER_H_
