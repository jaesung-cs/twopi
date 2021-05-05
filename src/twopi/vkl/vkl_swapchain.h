#ifndef TWOPI_VKL_VKL_SWAPCHAIN_H_
#define TWOPI_VKL_VKL_SWAPCHAIN_H_

#include <twopi/vkl/vkl_object.h>

namespace twopi
{
namespace vkl
{
class Swapchain : public Object
{
public:
  Swapchain() = delete;

  Swapchain(std::shared_ptr<vkl::Context> context, uint32_t width, uint32_t height);

  ~Swapchain() override;

  operator vk::SwapchainKHR() const { return swapchain_; }
  const auto& Images() const { return images_; }
  const auto& ImageViews() const { return image_views_; }
  auto ImageFormat() const { return image_format_; }
  auto ImageCount() const { return image_count_; }

private:
  uint32_t width_ = 0;
  uint32_t height_ = 0;
  uint32_t image_count_ = 3;

  vk::SwapchainKHR swapchain_;
  vk::Format image_format_;
  std::vector<vk::Image> images_;
  std::vector<vk::ImageView> image_views_;
};
}
}

#endif // TWOPI_VKL_VKL_SWAPCHAIN_H_
