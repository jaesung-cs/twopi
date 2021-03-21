#include <twopi/gl/gl_framebuffer.h>

#include <glad/glad.h>

#include <twopi/gl/gl_texture.h>
#include <twopi/gl/gl_renderbuffer.h>

namespace twopi
{
namespace gl
{
namespace impl
{
class FramebufferImpl
{
public:
  static void Unbind()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

public:
  FramebufferImpl()
  {
    glCreateFramebuffers(1, &framebuffer_);
  }

  ~FramebufferImpl()
  {
    glDeleteFramebuffers(1, &framebuffer_);
  }

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

  void Bind()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
  }

private:
  GLuint framebuffer_ = 0;
};
}

void Framebuffer::Unbind()
{
  impl::FramebufferImpl::Unbind();
}

Framebuffer::Framebuffer()
{
  impl_ = std::make_unique<impl::FramebufferImpl>();
}

Framebuffer::~Framebuffer() = default;

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

void Framebuffer::Bind()
{
  impl_->Bind();
}
}
}
