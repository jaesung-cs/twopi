#ifndef TWOPI_APPLICATION_APPLICATION_IMPL_H_
#define TWOPI_APPLICATION_APPLICATION_IMPL_H_

#include <memory>

namespace twopi
{
namespace impl
{
class ApplicationImpl
{
public:
  ApplicationImpl();
  ~ApplicationImpl();

  virtual void Run();

private:
};
}
}

#endif // TWOPI_APPLICATION_APPLICATION_IMPL_H_
