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

class Framebuffer
{
public:
  static void Unbind();

public:
  Framebuffer() = delete;
  Framebuffer(int width, int height);
  ~Framebuffer();

  int Width() const;
  int Height() const;

  void AttachColor(int attachment, std::shared_ptr<Texture> texture);
  void AttachDepthStencil(std::shared_ptr<Texture> texture);
  void AttachDepthStencil(std::shared_ptr<Renderbuffer> texture);
  bool IsComplete() const;

  void Bind();

  void BlitTo(std::shared_ptr<Framebuffer> framebuffer);
  void BlitTo(int read_attachment, std::shared_ptr<Framebuffer> framebuffer, int draw_attachment);

private:
  unsigned int Id() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_GL_GL_FRAMEBUFFER_H_
