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
private:
  static constexpr GLenum InternalFormat(int comp)
  {
    switch (comp)
    {
    case 1: return GL_R8;
    case 2: return GL_RG8;
    case 3: return GL_RGB8;
    case 4: return GL_RGBA8;
    default: return GL_RGBA8;
    }
  }

public:
  TextureImpl()
  {
  }

  ~TextureImpl()
  {
    if (texture_)
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
    if (!texture_)
      glCreateTextures(GL_TEXTURE_2D, 1, &texture_);

    const auto internal_format = InternalFormat(comp);
    glTextureStorage2D(texture_, 3, internal_format, width, height);
    glTextureParameteri(texture_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(texture_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  void StorageMultisample(int width, int height, int comp)
  {
    if (!texture_)
      glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &texture_);

    const auto internal_format = InternalFormat(comp);
    glTextureStorage2DMultisample(texture_, 4, internal_format, width, height, GL_TRUE);
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

void Texture::StorageMultisample(int width, int height, int comp)
{
  impl_->StorageMultisample(width, height, comp);
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
