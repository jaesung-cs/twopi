#ifndef TWOPI_GL_GL_RENDERBUFFER_H_
#define TWOPI_GL_GL_RENDERBUFFER_H_

#include <memory>
#include <vector>

namespace twopi
{
namespace gl
{
namespace impl
{
class RenderbufferImpl;
}

class Renderbuffer
{
public:
  Renderbuffer();
  ~Renderbuffer();

  void DepthStencilStorage(int width, int height);

  unsigned int Id() const;

private:
  std::unique_ptr<impl::RenderbufferImpl> impl_;
};
}
}

#endif // TWOPI_GL_GL_RENDERBUFFER_H_
