#include <twopi/gl/gl_texture.h>

#include <glad/glad.h>

#include <twopi/geometry/image.h>

namespace twopi
{
namespace gl
{
namespace impl
{
class TextureImpl
{
public:
  TextureImpl()
  {
    glCreateTextures(GL_TEXTURE_2D, 1, &texture_);
  }

  ~TextureImpl()
  {
    glDeleteTextures(1, &texture_);
  }

  template <typename T>
  void Load(std::shared_ptr<geometry::Image<T>> image);

  template <>
  void Load(std::shared_ptr<geometry::Image<uint8_t>> image)
  {
    const auto width = image->Width();
    const auto height = image->Height();
    const auto comp = image->Comp();

    Storage(width, height, comp);

    GLenum format = GL_RGBA;
    switch (image->Comp())
    {
    case 3:
      format = GL_RGB;
      break;
    }


    glTextureSubImage2D(texture_, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, image->Buffer().data());
    glGenerateTextureMipmap(texture_);
  }

  void Storage(int width, int height, int comp)
  {
    GLenum internal_format = GL_RGB8;
    switch (comp)
    {
    case 3:
      internal_format = GL_RGB8;
      break;
    }

    glTextureStorage2D(texture_, 3, internal_format, width, height);
    glTextureParameteri(texture_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(texture_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  void Bind(int unit)
  {
    glBindTextureUnit(unit, texture_);
  }

  auto Id() const
  {
    return texture_;
  }

private:
  GLuint texture_ = 0;
};
}

Texture::Texture()
{
  impl_ = std::make_unique<impl::TextureImpl>();
}

Texture::~Texture() = default;

template <typename T>
void Texture::Load(std::shared_ptr<geometry::Image<T>> image)
{
  impl_->Load(image);
}

void Texture::Storage(int width, int height, int comp)
{
  impl_->Storage(width, height, comp);
}

void Texture::Bind(int unit)
{
  impl_->Bind(unit);
}

GLuint Texture::Id() const
{
  return impl_->Id();
}

template void Texture::Load(std::shared_ptr<geometry::Image<uint8_t>> image);
}
}
