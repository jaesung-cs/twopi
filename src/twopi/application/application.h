#ifndef TWOPI_APPLICATION_APPLICATION_H_
#define TWOPI_APPLICATION_APPLICATION_H_

#include <memory>

namespace twopi
{
namespace app
{
namespace impl
{
class ApplicationImpl;
}

class Application
{
public:
  Application();
  ~Application();

  void Run();

private:
  std::unique_ptr<impl::ApplicationImpl> impl_;
};
}
}

#endif // TWOPI_APPLICATION_APPLICATION_H_
