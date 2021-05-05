#ifndef TWOPI_VKL_VKL_RENDERTARGET_H_
#define TWOPI_VKL_VKL_RENDERTARGET_H_

#include <twopi/vkl/vkl_object.h>
#include <twopi/vkl/vkl_memory.h>

namespace twopi
{
namespace vkl
{
class Rendertarget : public Object
{
public:
  Rendertarget() = delete;

  Rendertarget(std::shared_ptr<vkl::Context> context, uint32_t width, uint32_t height, vk::Format format, vk::SampleCountFlagBits samples);

  Rendertarget(std::shared_ptr<vkl::Context> context, uint32_t max_width, uint32_t max_height, uint32_t width, uint32_t height, vk::Format format, vk::SampleCountFlagBits samples);

  virtual ~Rendertarget() override;

  auto ColorImage() const { return color_image_; }
  auto ColorImageView() const { return color_image_view_; }
  auto DepthImage() const { return depth_image_; }
  auto DepthImageView() const { return depth_image_view_; }

private:
  vk::Image color_image_;
  vk::ImageView color_image_view_;
  Memory color_image_memory_;

  vk::Image depth_image_;
  vk::ImageView depth_image_view_;
  Memory depth_image_memory_;
};
}
}

#endif // TWOPI_VKL_VKL_RENDERTARGET_H_
