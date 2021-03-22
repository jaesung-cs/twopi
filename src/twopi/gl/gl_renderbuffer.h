#ifndef TWOPI_GL_GL_RENDERBUFFER_H_
#define TWOPI_GL_GL_RENDERBUFFER_H_

#include <memory>
#include <vector>

namespace twopi
{
namespace gl
{
class Renderbuffer
{
public:
  Renderbuffer();
  ~Renderbuffer();

  void DepthStencilStorage(int width, int height);
  void DepthStencilStorageMultisample(int width, int height);

  unsigned int Id() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_GL_GL_RENDERBUFFER_H_
