#include <twopi/gl/gl_framebuffer.h>

#include <glad/glad.h>

#include <twopi/gl/gl_texture.h>
#include <twopi/gl/gl_renderbuffer.h>

namespace twopi
{
namespace gl
{
class Framebuffer::Impl
{
public:
  static void Unbind()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

public:
  Impl(int width, int height)
    : width_(width), height_(height)
  {
    glCreateFramebuffers(1, &framebuffer_);
  }

  ~Impl()
  {
    glDeleteFramebuffers(1, &framebuffer_);
  }

  int Width() const { return width_; }
  int Height() const { return height_; }

  void AttachColor(int attachment, std::shared_ptr<Texture> texture)
  {
    glNamedFramebufferTexture(framebuffer_, GL_COLOR_ATTACHMENT0 + attachment, texture->Id(), 0);
  }

  void AttachDepthStencil(std::shared_ptr<Texture> texture)
  {
    glNamedFramebufferTexture(framebuffer_, GL_DEPTH_STENCIL_ATTACHMENT, texture->Id(), 0);
  }

  void AttachDepthStencil(std::shared_ptr<Renderbuffer> renderbuffer)
  {
    glNamedFramebufferRenderbuffer(framebuffer_, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer->Id());
  }

  bool IsComplete() const
  {
    return glCheckNamedFramebufferStatus(framebuffer_, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
  }

  void Bind()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
  }

  void BlitTo(std::shared_ptr<Framebuffer> framebuffer)
  {
    BlitTo(0, framebuffer, 0);
  }

  void BlitTo(int read_attachment, std::shared_ptr<Framebuffer> framebuffer, int draw_attachment)
  {
    glNamedFramebufferDrawBuffer(framebuffer_, GL_COLOR_ATTACHMENT0 + read_attachment);
    glNamedFramebufferDrawBuffer(framebuffer->Id(), GL_COLOR_ATTACHMENT0 + draw_attachment);
    glBlitNamedFramebuffer(framebuffer_, framebuffer->Id(), 0, 0, width_, height_, 0, 0, framebuffer->Width(), framebuffer->Height(), GL_COLOR_BUFFER_BIT, GL_LINEAR);
  }

  auto Id() const
  {
    return framebuffer_;
  }

private:
  int width_ = 1;
  int height_ = 1;

  GLuint framebuffer_ = 0;
};

void Framebuffer::Unbind()
{
  Impl::Unbind();
}

Framebuffer::Framebuffer(int width, int height)
{
  impl_ = std::make_unique<Impl>(width, height);
}

Framebuffer::~Framebuffer() = default;

int Framebuffer::Width() const
{
  return impl_->Width();
}

int Framebuffer::Height() const
{
  return impl_->Height();
}

void Framebuffer::AttachColor(int attachment, std::shared_ptr<Texture> texture)
{
  impl_->AttachColor(attachment, texture);
}

void Framebuffer::AttachDepthStencil(std::shared_ptr<Texture> texture)
{
  impl_->AttachDepthStencil(texture);
}

void Framebuffer::AttachDepthStencil(std::shared_ptr<Renderbuffer> renderbuffer)
{
  impl_->AttachDepthStencil(renderbuffer);
}

bool Framebuffer::IsComplete() const
{
  return impl_->IsComplete();
}

void Framebuffer::Bind()
{
  impl_->Bind();
}

void Framebuffer::BlitTo(std::shared_ptr<Framebuffer> framebuffer)
{
  impl_->BlitTo(framebuffer);
}

void Framebuffer::BlitTo(int read_attachment, std::shared_ptr<Framebuffer> framebuffer, int draw_attachment)
{
  impl_->BlitTo(read_attachment, framebuffer, draw_attachment);
}

GLuint Framebuffer::Id() const
{
  return impl_->Id();
}
}
}
