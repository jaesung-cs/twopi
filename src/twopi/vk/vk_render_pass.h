#ifndef TWOPI_VK_VK_RENDER_PASS_H_
#define TWOPI_VK_VK_RENDER_PASS_H_

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkw
{
class Device;
class Image;

class RenderPass
{
public:
  class Creator
  {
  public:
    Creator() = delete;
    Creator(Device device);
    ~Creator();

    Creator& SetFormat(Image image);
    Creator& SetMultisample4();

    RenderPass Create();

  private:
    const vk::Device device_;

    vk::RenderPassCreateInfo create_info_;
    vk::AttachmentDescription color_attachment_;
    vk::AttachmentReference color_attachment_ref_;
    vk::AttachmentDescription depth_attachment_;
    vk::AttachmentReference depth_attachment_ref_;
    vk::SubpassDescription subpass_;
    vk::SubpassDependency dependency_;
    vk::Format format_;

    bool has_color_attachment_resolve_ = false;
    vk::AttachmentDescription color_attachment_resolve_;
    vk::AttachmentReference color_attachment_resolve_ref_;
  };

public:
  RenderPass();
  RenderPass(vk::Device device, vk::RenderPass render_pas);

  ~RenderPass();
  
  void Destroy();

  operator vk::RenderPass() const;

private:
  vk::Device device_;
  vk::RenderPass render_pass_;
};
}
}

#endif // TWOPI_VK_VK_RENDER_PASS_H_
