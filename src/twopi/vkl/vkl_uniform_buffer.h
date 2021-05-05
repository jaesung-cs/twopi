#ifndef TWOPI_VKL_VKL_UNIFORM_BUFFER_H_
#define TWOPI_VKL_VKL_UNIFORM_BUFFER_H_

#include <cstring>

#include <twopi/vkl/vkl_object.h>
#include <twopi/vkl/vkl_memory.h>

namespace twopi
{
namespace vkl
{
class Context;

class UniformBuffer : public Object
{
public:
  template <typename UniformStructType>
  class Uniform
  {
  public:
    Uniform() = delete;

    Uniform(UniformBuffer& buffer, vk::DeviceSize offset, uint32_t stride)
      : buffer_{ buffer }
      , offset_{ offset }
      , stride_{ stride }
    {
    }

    ~Uniform()
    {
    }

    auto Offset() const { return offset_; }
    auto Stride() const { return stride_; }

    Uniform operator [] (int index)
    {
      // TODO: do not allow chaining brackets
      return Uniform(buffer_, offset_ + stride_ * index, stride_);
    }

    Uniform& operator = (const UniformStructType& rhs)
    {
      std::memcpy(buffer_.map_ + offset_, &rhs, sizeof(UniformStructType));
      return *this;
    }

  private:
    UniformBuffer& buffer_;
    vk::DeviceSize offset_ = 0;
    uint32_t stride_ = 0;
  };

public:
  UniformBuffer() = delete;

  explicit UniformBuffer(std::shared_ptr<vkl::Context> context);

  ~UniformBuffer();

  auto Buffer() const { return buffer_; }

  template <typename UniformStructType>
  Uniform<UniformStructType> Allocate(int count = 1)
  {
    const auto stride = Stride<UniformStructType>();

    Uniform<UniformStructType> uniform{ *this, allocation_offset_, stride };
    allocation_offset_ += stride * count;
    return uniform;
  }

private:
  vk::DeviceSize ubo_alignment_ = 0;

  template <typename T>
  uint32_t Stride() const
  {
    const auto alignment = static_cast<uint32_t>(ubo_alignment_);
    return (static_cast<uint32_t>(sizeof(T)) + alignment -1u) & ~(alignment - 1u);
  }

  vk::Buffer buffer_;
  Memory memory_;
  unsigned char* map_ = nullptr;

  uint64_t allocation_offset_ = 0;
};
}
}

#endif // TWOPI_VKL_VKL_UNIFORM_BUFFER_H_
