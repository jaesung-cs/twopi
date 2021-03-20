#include <twopi/geometry/image_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <twopi/geometry/image.h>

namespace twopi
{
namespace geometry
{
namespace impl
{
class ImageLoaderImpl
{
public:
  ImageLoaderImpl()
  {
  }

  ~ImageLoaderImpl() = default;

  template <typename T>
  std::shared_ptr<Image<T>> Load(const std::string& filepath);

  template <>
  std::shared_ptr<Image<uint8_t>> Load(const std::string& filepath)
  {
    int width = 1;
    int height = 1;
    int comp = 1;

    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &comp, 0);

    const auto image = std::make_shared<Image<uint8_t>>(height, width, comp);
    image->CopyBuffer(data);
    stbi_image_free(data);

    return image;
  }

private:
};
}

ImageLoader::ImageLoader()
{
  impl_ = std::make_unique<impl::ImageLoaderImpl>();
}

ImageLoader::~ImageLoader() = default;

template <typename T>
std::shared_ptr<Image<T>> ImageLoader::Load(const std::string& filepath)
{
  return impl_->Load<T>(filepath);
}

template std::shared_ptr<Image<uint8_t>> ImageLoader::Load(const std::string& filepath);
}
}