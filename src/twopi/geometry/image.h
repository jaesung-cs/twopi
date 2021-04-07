#ifndef TWOPI_GEOMETRY_IMAGE_H_
#define TWOPI_GEOMETRY_IMAGE_H_

#include <memory>
#include <vector>

namespace twopi
{
namespace geometry
{
template <typename T>
class Image
{
public:
  Image();
  Image(int wodtj, int height, int comp);
  ~Image();

  void CopyBuffer(const T* const buffer);

  uint32_t Width() const;
  uint32_t Height() const;
  int Comp() const;
  const std::vector<T>& Buffer() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_GEOMETRY_IMAGE_H_
