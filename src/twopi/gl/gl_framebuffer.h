#ifndef TWOPI_GL_GL_FRAMEBUFFER_H_
#define TWOPI_GL_GL_FRAMEBUFFER_H_

#include <memory>
#include <vector>

namespace twopi
{
namespace gl
{
class Texture;
class Renderbuffer;

namespace impl
{
class FramebufferImpl;
}

class Framebuffer
{
public:
  static void Unbind();

public:
  Framebuffer();
  ~Framebuffer();

  void AttachColor(int attachment, std::shared_ptr<Texture> texture);
  void AttachDepthStencil(std::shared_ptr<Texture> texture);
  void AttachDepthStencil(std::shared_ptr<Renderbuffer> texture);

  void Bind();

private:
  std::unique_ptr<impl::FramebufferImpl> impl_;
};
}
}

#endif // TWOPI_GL_GL_FRAMEBUFFER_H_
