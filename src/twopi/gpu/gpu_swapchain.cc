#include <twopi/gpu/gpu_swapchain.h>

#include <vulkan/vulkan.hpp>

#include <twopi/gpu/gpu_device.h>

namespace twopi
{
namespace gpu
{
class Swapchain::Impl
{
public:
  Impl() = delete;

  Impl(std::shared_ptr<Device> device, uint32_t width, uint32_t height)
    : device_(device)
    , width_(width)
    , height_(height)
  {
    Create();
  }

  ~Impl()
  {
    Destroy();
  }

  const std::vector<vk::ImageView>& ImageViews() const
  {
    return image_views_;
  }

  uint32_t ImageCount() const
  {
    return image_views_.size();
  }

  vk::Format Format() const
  {
    return format_.format;
  }

  void Resize(uint32_t width, uint32_t height)
  {
    width_ = width;
    height_ = height;

    Destroy();
    Create();
  }

  std::pair<uint32_t, vk::Result> AcquireNextImage(vk::Semaphore semaphore)
  {
    auto result = device_->DeviceHandle().acquireNextImageKHR(swapchain_, UINT64_MAX, semaphore);
    return std::make_pair(result.value, result.result);
  }

private:
  void Create()
  {
    vk::PhysicalDevice physical_device = device_->PhysicalDevice();
    vk::Device device = device_->DeviceHandle();
    vk::SurfaceKHR surface = device_->Surface();

    const auto available_formats = physical_device.getSurfaceFormatsKHR(surface);
    auto format = available_formats[0];
    for (const auto& available_format : available_formats)
    {
      if (available_format.format == vk::Format::eB8G8R8A8Srgb && available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        format = available_format;
    }

    const auto capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);
    uint32_t image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
      image_count = capabilities.maxImageCount;

    vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo;
    const auto present_modes = physical_device.getSurfacePresentModesKHR(surface);
    for (auto available_mode : present_modes)
    {
      if (available_mode == vk::PresentModeKHR::eMailbox)
        present_mode = vk::PresentModeKHR::eMailbox;
    }

    swapchain_ = device.createSwapchainKHR(vk::SwapchainCreateInfoKHR{
      {}, surface,
      image_count, format.format, format.colorSpace,
      { width_, height_ }, 1,
      vk::ImageUsageFlagBits::eColorAttachment,
      vk::SharingMode::eConcurrent, device_->QueueFamilies(),
      vk::SurfaceTransformFlagBitsKHR::eIdentity,
      vk::CompositeAlphaFlagBitsKHR::eOpaque, present_mode,
      VK_TRUE, nullptr
      });

    images_ = device.getSwapchainImagesKHR(swapchain_);
    for (int i = 0; i < images_.size(); i++)
    {
      image_views_.emplace_back(device.createImageView({
        {}, images_[i],
        vk::ImageViewType::e2D,
        format.format, vk::ComponentMapping{},
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
        }));
    }

    format_ = format;
  }

  void Destroy()
  {
    const auto device = device_->DeviceHandle();

    for (auto& image_view : image_views_)
      device.destroyImageView(image_view);
    image_views_.clear();

    device.destroySwapchainKHR(swapchain_);
  }

  const std::shared_ptr<Device> device_;

  uint32_t width_;
  uint32_t height_;
  vk::SwapchainKHR swapchain_;
  std::vector<vk::Image> images_;
  std::vector<vk::ImageView> image_views_;
  vk::SurfaceFormatKHR format_;
};

Swapchain::Swapchain(std::shared_ptr<Device> device, uint32_t width, uint32_t height)
  : impl_(std::make_unique<Impl>(device, width, height))
{
}

Swapchain::~Swapchain() = default;

const std::vector<vk::ImageView>& Swapchain::ImageViews() const
{
  return impl_->ImageViews();
}

uint32_t Swapchain::ImageCount() const
{
  return impl_->ImageCount();
}

vk::Format Swapchain::Format() const
{
  return impl_->Format();
}

void Swapchain::Resize(uint32_t width, uint32_t height)
{
  impl_->Resize(width, height);
}

std::pair<uint32_t, vk::Result> Swapchain::AcquireNextImage(vk::Semaphore semaphore)
{
  return impl_->AcquireNextImage(semaphore);
}
}
}
