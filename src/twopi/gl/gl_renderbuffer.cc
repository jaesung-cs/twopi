#include <twopi/gl/gl_renderbuffer.h>

#include <glad/glad.h>

namespace twopi
{
namespace gl
{
namespace impl
{
class RenderbufferImpl
{
public:
  RenderbufferImpl()
  {
    glCreateRenderbuffers(1, &renderbuffer_);
  }

  ~RenderbufferImpl()
  {
    glDeleteRenderbuffers(1, &renderbuffer_);
  }

  void DepthStencilStorage(int width, int height)
  {
    glNamedRenderbufferStorage(renderbuffer_, GL_DEPTH24_STENCIL8, width, height);
  }

  void DepthStencilStorageMultisample(int width, int height)
  {
    glNamedRenderbufferStorageMultisample(renderbuffer_, 4, GL_DEPTH24_STENCIL8, width, height);
  }

  auto Id() const
  {
    return renderbuffer_;
  }

private:
  GLuint renderbuffer_ = 0;
};
}

Renderbuffer::Renderbuffer()
{
  impl_ = std::make_unique<impl::RenderbufferImpl>();
}

Renderbuffer::~Renderbuffer() = default;

void Renderbuffer::DepthStencilStorage(int width, int height)
{
  impl_->DepthStencilStorage(width, height);
}

void Renderbuffer::DepthStencilStorageMultisample(int width, int height)
{
  impl_->DepthStencilStorageMultisample(width, height);
}

GLuint Renderbuffer::Id() const
{
  return impl_->Id();
}
}
}
