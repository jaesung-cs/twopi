#ifndef TWOPI_WINDOW_WINDOW_H_
#define TWOPI_WINDOW_WINDOW_H_

#include <vector>
#include <memory>

#include <twopi/core/timestamp.h>

namespace twopi
{
namespace window
{
class Event;

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
  virtual std::vector<std::shared_ptr<Event>> PollEvents(core::Timestamp timestamp) = 0;
  virtual void SwapBuffers() = 0;

protected:
  void Resized(int width, int height);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_WINDOW_WINDOW_H_
