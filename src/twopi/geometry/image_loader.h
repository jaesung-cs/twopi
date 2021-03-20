#ifndef TWOPI_GEOMETRY_IMAGE_LOADER_H_
#define TWOPI_GEOMETRY_IMAGE_LOADER_H_

#include <memory>
#include <string>

namespace twopi
{
namespace geometry
{
template <typename T>
class Image;

namespace impl
{
class ImageLoaderImpl;
}

class ImageLoader
{
public:
  ImageLoader();
  ~ImageLoader();

  template <typename T>
  std::shared_ptr<Image<T>> Load(const std::string& filepath);

private:
  std::unique_ptr<impl::ImageLoaderImpl> impl_;
};
}
}

#endif // TWOPI_GEOMETRY_IMAGE_LOADER_H_
