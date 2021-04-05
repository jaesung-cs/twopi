#include <twopi/vke/vke_image.h>

#include <twopi/vkw/vkw_image.h>
#include <twopi/vkw/vkw_device_memory.h>
#include <twopi/vke/vke_memory.h>

namespace twopi
{
namespace vke
{
class Image::Impl
{
public:
  Impl() = delete;

  Impl(vkw::Image&& image, vkw::DeviceMemory memory, uint64_t offset, uint64_t size)
    : Impl(std::move(image), Memory(memory, offset, size)) {}

  Impl(vkw::Image&& image, Memory memory)
    : image_(std::move(image))
    , memory_(memory)
  {
    image_.Bind(memory_.DeviceMemory(), memory_.Offset());
  }

  ~Impl()
  {
    image_.Destroy();
  }

  operator vkw::Image() const
  {
    return image_;
  }

  uint64_t Offset() const
  {
    return memory_.Offset();
  }

  void* Map()
  {
    return memory_.DeviceMemory().Map(memory_.Offset(), memory_.Size());
  }

  void Unmap()
  {
    memory_.DeviceMemory().Unmap();
  }

private:
  vkw::Image image_;
  Memory memory_;
};

Image::Image(vkw::Image&& image, vkw::DeviceMemory memory, uint64_t offset, uint64_t size)
  : impl_(std::make_unique<Impl>(std::move(image), memory, offset, size))
{
}

Image::Image(vkw::Image&& image, Memory memory)
  : impl_(std::make_unique<Impl>(std::move(image), memory))
{
}

Image::~Image() = default;

Image::operator vkw::Image() const
{
  return impl_->operator vkw::Image();
}

uint64_t Image::Offset() const
{
  return impl_->Offset();
}

void* Image::Map()
{
  return impl_->Map();
}

void Image::Unmap()
{
  impl_->Unmap();
}
}
}
