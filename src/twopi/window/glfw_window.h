#ifndef TWOPI_WINDOW_GLFW_WINDOW_H_
#define TWOPI_WINDOW_GLFW_WINDOW_H_

#include <twopi/window/window.h>

#include <memory>

struct GLFWwindow;

namespace twopi
{
namespace window
{
class GlfwWindow : public Window
{
public:
  GlfwWindow();
  ~GlfwWindow() override;

  GLFWwindow* Handle() const;

  void Close() override;
  bool ShouldClose() const override;
  std::vector<std::shared_ptr<Event>> PollEvents() override;
  std::vector<std::shared_ptr<Event>> PollEvents(core::Timestamp timestamp) override;
  void SwapBuffers() override;

  int MaxWidth() const override;
  int MaxHeight() const override;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_WINDOW_GLFW_WINDOW_H_
