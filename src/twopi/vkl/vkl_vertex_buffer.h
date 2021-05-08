#ifndef TWOPI_VKL_VKL_VERTEX_BUFFER_H_
#define TWOPI_VKL_VKL_VERTEX_BUFFER_H_

#include <cstring>
#include <iostream>
#include <vector>

#include <twopi/vkl/vkl_object.h>
#include <twopi/vkl/vkl_memory.h>

namespace twopi
{
namespace vkl
{
class Context;

class VertexBuffer : public Object
{
private:
  struct Attribute
  {
    int index;
    int byte_size;
    int size;
  };

public:
  VertexBuffer() = delete;

  VertexBuffer(std::shared_ptr<vkl::Context> context, int num_vertices, int num_indices);

  ~VertexBuffer() override;

  template <typename T, int size>
  VertexBuffer& AddAttribute(int index)
  {
    Attribute attribute;
    attribute.index = index;
    attribute.byte_size = sizeof(T);
    attribute.size = size;
    attributes_.push_back(attribute);

    return *this;
  }

  void Prepare();

  int NumIndices() const { return num_indices_; }

  vk::DeviceSize IndexOffset() const;
  vk::DeviceSize IndexSize() const;
  vk::DeviceSize Offset(int index) const;
  vk::DeviceSize Size(int index) const;

  auto BufferSize() const { return buffer_size_; }

  auto Buffer() const { return buffer_; }

private:
  int num_vertices_;
  int num_indices_;

  std::vector<Attribute> attributes_;
  std::vector<vk::DeviceSize> offsets_;
  std::vector<vk::DeviceSize> sizes_;
  vk::DeviceSize index_offset_ = 0;
  vk::DeviceSize index_size_ = 0;
  vk::DeviceSize buffer_size_ = 0;
  vk::Buffer buffer_;
  Memory memory_;
};
}
}

#endif // TWOPI_VKL_VKL_VERTEX_BUFFER_H_
