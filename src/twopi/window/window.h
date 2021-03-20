#ifndef TWOPI_WINDOW_WINDOW_H_
#define TWOPI_WINDOW_WINDOW_H_

#include <twopi/window/event/event.h>

#include <vector>
#include <memory>

namespace twopi
{
namespace window
{
namespace impl
{
class WindowImpl;
}

class Window
{
public:
  Window();
  virtual ~Window();

  int Width() const;
  int Height() const;

  virtual void Close() = 0;
  virtual bool ShouldClose() const = 0;
  virtual std::vector<std::shared_ptr<Event>> PollEvents() = 0;
  virtual void SwapBuffers() = 0;

private:
  std::unique_ptr<impl::WindowImpl> impl_;
};
}
}

#endif // TWOPI_WINDOW_WINDOW_H_
