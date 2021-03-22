#ifndef TWOPI_APPLICATION_APPLICATION_H_
#define TWOPI_APPLICATION_APPLICATION_H_

#include <memory>

namespace twopi
{
namespace app
{
class Application
{
public:
  Application();
  ~Application();

  void Run();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_APPLICATION_APPLICATION_H_
