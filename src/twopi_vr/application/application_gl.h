#ifndef TWOPI_APPLICATION_APPLICATION_GL_H_
#define TWOPI_APPLICATION_APPLICATION_GL_H_

#include <twopi/application/application_impl.h>

namespace twopi
{
namespace impl
{
class ApplicationGl : public ApplicationImpl
{
public:
  ApplicationGl();
  ~ApplicationGl();

  void Run() override;

private:
};
}
}

#endif // TWOPI_APPLICATION_APPLICATION_GL_H_
