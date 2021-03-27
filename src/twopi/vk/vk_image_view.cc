#include <twopi/vk/vk_image_view.h>

#include <twopi/vk/vk_device.h>
#include <twopi/vk/vk_image.h>

namespace twopi
{
namespace vkw
{
//
// Creator
//
ImageView::Creator::Creator(Device device)
  : device_(device)
{
  create_info_
    .setViewType(vk::ImageViewType::e2D)
    .setComponents(vk::ComponentMapping{})
    .setSubresourceRange(
      vk::ImageSubresourceRange{}
      .setAspectMask(vk::ImageAspectFlagBits::eColor)
      .setBaseMipLevel(0)
      .setLevelCount(1)
      .setBaseArrayLayer(0)
      .setLayerCount(1));
}

ImageView::Creator::~Creator() = default;

ImageView::Creator& ImageView::Creator::SetImage(Image image)
{
  create_info_
    .setImage(image)
    .setFormat(image.Format()); // Assume image is created with its format
  return *this;
}

ImageView ImageView::Creator::Create()
{
  auto image_view_handle = device_.createImageView(create_info_);
  return ImageView{ device_, image_view_handle };
}

//
// ImageView
//
ImageView::ImageView()
{
}

ImageView::ImageView(vk::Device device, vk::ImageView image_view)
  : device_(device), image_view_(image_view)
{
}

ImageView::~ImageView() = default;

ImageView::operator vk::ImageView() const
{
  return image_view_;
}

void ImageView::Destroy()
{
  device_.destroyImageView(image_view_);
}
}
}
