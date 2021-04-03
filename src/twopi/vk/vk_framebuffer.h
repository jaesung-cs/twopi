#ifndef TWOPI_VK_VK_FRAMEBUFFER_H_
#define TWOPI_VK_VK_FRAMEBUFFER_H_

#include <initializer_list>
#include <vector>

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

    Creator& SetAttachments(std::initializer_list<ImageView> image_views);
    Creator& SetAttachment(ImageView image_view);
    Creator& SetExtent(int width, int height);
    Creator& SetRenderPass(RenderPass render_pass);

    Framebuffer Create();

  private:
    const vk::Device device_;

    vk::FramebufferCreateInfo create_info_{};
    std::vector<vk::ImageView> image_views_;
    uint32_t width_ = 0;
    uint32_t height_ = 0;
  };

public:
  Framebuffer();
  Framebuffer(vk::Device device, vk::Framebuffer framebuffer);

  ~Framebuffer();

  void Destroy();

  operator vk::Framebuffer() const;

  auto Width() const { return width_; }
  auto Height() const { return height_; }

private:
  vk::Device device_;
  vk::Framebuffer framebuffer_;
  uint32_t width_ = 0;
  uint32_t height_ = 0;
};
}
}

#endif // TWOPI_VK_VK_FRAMEBUFFER_H_
