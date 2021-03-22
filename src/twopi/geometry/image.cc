#include <twopi/geometry/image.h>

#include <iterator>
#include <algorithm>

namespace twopi
{
namespace geometry
{
template <typename T>
class Image<T>::Impl
{
public:
  Impl()
  {
  }

  Impl(int width, int height, int comp)
    : width_(width), height_(height), comp_(comp)
  {
  }

  ~Impl()
  {
  }

  void CopyBuffer(const T* const buffer)
  {
    buffer_.clear();

    // Pixels in buffer are stored from top-left, left-to-right, to bottom-right.
    // Store this image from bottom-left, left-to-right, to top-right, by flipping the scanline order
    for (int r = 0; r < height_; r++)
      std::copy(buffer + (height_ - r - 1) * width_ * comp_, buffer + (height_ - r) * width_ * comp_, std::back_inserter(buffer_));
  }

  int Width() const
  {
    return width_;
  }

  int Height() const
  {
    return height_;
  }

  int Comp() const
  {
    return comp_;
  }

  const std::vector<T>& Buffer() const
  {
    return buffer_;
  }

private:
  int width_ = 1;
  int height_ = 1;
  int comp_ = 1;

  std::vector<T> buffer_;
};

template <typename T>
Image<T>::Image()
{
  impl_ = std::make_unique<Impl>();
}

template <typename T>
Image<T>::Image(int width, int height, int comp)
{
  impl_ = std::make_unique<Impl>(width, height, comp);
}

template <typename T>
Image<T>::~Image() = default;

template <typename T>
void Image<T>::CopyBuffer(const T* const buffer)
{
  impl_->CopyBuffer(buffer);
}

template <typename T>
int Image<T>::Width() const
{
  return impl_->Width();
}

template <typename T>
int Image<T>::Height() const
{
  return impl_->Height();
}

template <typename T>
int Image<T>::Comp() const
{
  return impl_->Comp();
}

template <typename T>
const std::vector<T>& Image<T>::Buffer() const
{
  return impl_->Buffer();
}

// Template instantiation
template class Image<uint8_t>;
template class Image<float>;
}
}
