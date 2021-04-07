#include <twopi/vke/vke_image.h>

#include <twopi/vke/vke_context.h>
#include <twopi/vke/vke_memory.h>
#include <twopi/vke/vke_memory_manager.h>

namespace twopi
{
namespace vke
{
Image::Image(std::shared_ptr<Context> context, vk::ImageCreateInfo create_info, MemoryType memory_type)
  : context_(context)
{
  width_ = create_info.extent.width;
  height_ = create_info.extent.height;

  format_ = create_info.format;
  mip_levels_ = create_info.mipLevels;

  image_ = context_->Device().createImage(create_info);

  switch (memory_type)
  {
  case MemoryType::Host:
    memory_ = std::make_unique<Memory>(context_->MemoryManager()->AllocateHostVisibleMemory(image_));
    break;
  case MemoryType::Device:
    memory_ = std::make_unique<Memory>(context_->MemoryManager()->AllocateDeviceLocalMemory(image_));
    break;
  }

  context_->Device().bindImageMemory(image_, *memory_, memory_->Offset());
}

Image::~Image()
{
  context_->Device().destroyImage(image_);
}

Image::operator vk::Image() const
{
  return image_;
}

void* Image::Map()
{
  return context_->Device().mapMemory(*memory_, memory_->Offset(), memory_->Size());
}

void Image::Unmap()
{
  context_->Device().unmapMemory(*memory_);
}
}
}
