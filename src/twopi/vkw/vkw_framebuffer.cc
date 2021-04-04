#include <twopi/vkw/vkw_framebuffer.h>

#include <twopi/vkw/vkw_device.h>
#include <twopi/vkw/vkw_render_pass.h>
#include <twopi/vkw/vkw_image_view.h>

namespace twopi
{
namespace vkw
{
//
// Creator
//
Framebuffer::Creator::Creator(Device device)
  : device_(device)
{
  create_info_.setLayers(1);
}

Framebuffer::Creator::~Creator() = default;

Framebuffer::Creator& Framebuffer::Creator::SetAttachments(std::initializer_list<ImageView> image_views)
{
  image_views_ = std::vector<vk::ImageView>(image_views.begin(), image_views.end());
  return *this;
}

Framebuffer::Creator& Framebuffer::Creator::SetAttachment(ImageView image_view)
{
  image_views_ = { image_view };
  return *this;
}

Framebuffer::Creator& Framebuffer::Creator::SetExtent(int width, int height)
{
  width_ = width;
  height_ = height;

  create_info_
    .setWidth(width)
    .setHeight(height);

  return *this;
}

Framebuffer::Creator& Framebuffer::Creator::SetRenderPass(RenderPass render_pass)
{
  create_info_.setRenderPass(render_pass);
  return *this;
}

Framebuffer Framebuffer::Creator::Create()
{
  create_info_.setAttachments(image_views_);

  const auto handle = device_.createFramebuffer(create_info_);
  auto framebuffer = Framebuffer{ device_, handle };
  framebuffer.width_ = width_;
  framebuffer.height_ = height_;
  return framebuffer;
}

//
// Framebuffer
//
Framebuffer::Framebuffer()
{
}

Framebuffer::Framebuffer(vk::Device device, vk::Framebuffer framebuffer)
  : device_(device), framebuffer_(framebuffer)
{
}

Framebuffer::~Framebuffer() = default;

void Framebuffer::Destroy()
{
  device_.destroyFramebuffer(framebuffer_);
}

Framebuffer::operator vk::Framebuffer() const
{
  return framebuffer_;
}
}
}
