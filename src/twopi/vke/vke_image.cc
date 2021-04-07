#include <twopi/vke/vke_image.h>

#include <twopi/vke/vke_context.h>
#include <twopi/vke/vke_memory.h>
#include <twopi/vke/vke_memory_manager.h>

namespace twopi
{
namespace vke
{
Image::Image(std::shared_ptr<vke::Context> context, vk::ImageCreateInfo create_info, MemoryType memory_type)
  : context_(context)
{
  width_ = create_info.extent.width;
  height_ = create_info.extent.height;

  format_ = create_info.format;
  mip_levels_ = create_info.mipLevels;

  image_ = context->Device().createImage(create_info);

  switch (memory_type)
  {
  case MemoryType::Host:
    memory_ = std::make_unique<Memory>(context->MemoryManager()->AllocateHostVisibleMemory(image_));
    break;
  case MemoryType::Device:
    memory_ = std::make_unique<Memory>(context->MemoryManager()->AllocateDeviceLocalMemory(image_));
    break;
  }

  context->Device().bindImageMemory(image_, *memory_, memory_->Offset());
}

Image::~Image()
{
  Context()->Device().destroyImage(image_);
}

Image::operator vk::Image() const
{
  return image_;
}

void* Image::Map()
{
  return Context()->Device().mapMemory(*memory_, memory_->Offset(), memory_->Size());
}

void Image::Unmap()
{
  Context()->Device().unmapMemory(*memory_);
}

std::shared_ptr<Context> Image::Context() const
{
  return context_.lock();
}
}
}
