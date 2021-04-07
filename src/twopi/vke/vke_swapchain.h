#ifndef TWOPI_VKE_VKE_SWAPCHAIN_H_
#define TWOPI_VKE_VKE_SWAPCHAIN_H_

#include <memory>

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vke
{
class Context;
class Image;

class Swapchain
{
public:
  Swapchain() = delete;
  explicit Swapchain(std::shared_ptr<Context> context, uint32_t width, uint32_t height);
  ~Swapchain();

  vk::Format Format() const;
  uint32_t Width() const;
  uint32_t Height() const;
  uint32_t ImageCount() const;

private:
  std::shared_ptr<Context> Context() const;

  std::weak_ptr<vke::Context> context_;

  vk::SwapchainKHR swapchain_;

  vk::Format format_;
  uint32_t width_;
  uint32_t height_;
  uint32_t image_count_;

  std::vector<vk::Framebuffer> swapchain_framebuffers_;

  std::vector<vk::Image> swapchain_images_;
  std::vector<vk::ImageView> swapchain_image_views_;

  std::unique_ptr<Image> multisample_color_image_;
  vk::ImageView multisample_color_image_view_;

  std::unique_ptr<Image> multisample_depth_image_;
  vk::ImageView multisample_depth_image_view_;
};
}
}

#endif // TWOPI_VKE_VKE_SWAPCHAIN_H_
