#include <twopi/window/glfw_window.h>

#include <memory>
#include <iostream>

#include <GLFW/glfw3.h>

#include <twopi/window/window.h>
#include <twopi/window/event/types.h>
#include <twopi/window/event/keyboard_event.h>
#include <twopi/window/event/mouse_button_event.h>
#include <twopi/window/event/mouse_move_event.h>
#include <twopi/window/event/mouse_wheel_event.h>
#include <twopi/core/error.h>

namespace twopi
{
namespace window
{
namespace impl
{
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void cursor_pos_callback(GLFWwindow* window, double x, double y);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

class GlfwWindowImpl
{
public:
  GlfwWindowImpl() = delete;

  GlfwWindowImpl(GlfwWindow* base)
    : base_(base)
  {
    if (!glfwInit())
    {
      throw core::Error("Failed to initialize GLFW.");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    window_ = glfwCreateWindow(base_->Width(), base_->Height(), "Twopi", NULL, NULL);

    glfwSetWindowUserPointer(window_, this);
    glfwSetMouseButtonCallback(window_, mouse_button_callback);
    glfwSetCursorPosCallback(window_, cursor_pos_callback);
    glfwSetKeyCallback(window_, key_callback);
    glfwSetScrollCallback(window_, scroll_callback);

    glfwSetWindowPos(window_, 100, 100);

    glfwMakeContextCurrent(window_);
  }

  ~GlfwWindowImpl()
  {
    glfwTerminate();
  }

  void Close()
  {
    glfwSetWindowShouldClose(window_, GLFW_TRUE);
  }

  bool ShouldClose() const
  {
    return glfwWindowShouldClose(window_);
  }

  std::vector<std::shared_ptr<Event>> PollEvents()
  {
    event_queue_.clear();
    glfwPollEvents();

    return event_queue_;
  }

  void SwapBuffers()
  {
    glfwSwapBuffers(window_);
  }

  void MouseButton(int button, int action, int mods)
  {
    window::MouseButton event_button;
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
      event_button = MouseButton::LEFT;
      break;

    case GLFW_MOUSE_BUTTON_RIGHT:
      event_button = MouseButton::RIGHT;
      break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
      event_button = MouseButton::MIDDLE;
      break;
    }

    MouseButtonState event_action;
    switch (action)
    {
    case GLFW_PRESS:
      event_action = MouseButtonState::PRESSED;
      break;

    case GLFW_RELEASE:
      event_action = MouseButtonState::RELEASED;
      break;
    }

    double xpos, ypos;
    glfwGetCursorPos(window_, &xpos, &ypos);

    const auto x = static_cast<int>(xpos);
    const auto y = static_cast<int>(ypos);

    const auto event = std::make_shared<MouseButtonEvent>(event_button, event_action, x, y);
    event_queue_.push_back(event);
  }

  void Key(int key, int scancode, int action, int mods)
  {
    KeyState event_action;
    switch (action)
    {
    case GLFW_PRESS:
      event_action = KeyState::PRESSED;
      break;

    case GLFW_RELEASE:
      event_action = KeyState::RELEASED;
      break;
    }

    const auto event = std::make_shared<KeyboardEvent>(key, event_action);
    event_queue_.push_back(event);
  }

  void CursorPos(double x, double y)
  {
    const auto xi = static_cast<int>(x);
    const auto yi = static_cast<int>(y);

    const auto event = std::make_shared<MouseMoveEvent>(xi, yi);
    event_queue_.push_back(event);
  }

  void Scroll(double scroll)
  {
    const auto scrolli = static_cast<int>(scroll);

    const auto event = std::make_shared<MouseWheelEvent>(scroll);
    event_queue_.push_back(event);
  }

private:
  const GlfwWindow* base_;
  GLFWwindow* window_;

  std::vector<std::shared_ptr<Event>> event_queue_;
};

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  auto module_window = static_cast<GlfwWindowImpl*>(glfwGetWindowUserPointer(window));
  module_window->MouseButton(button, action, mods);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  auto module_window = static_cast<GlfwWindowImpl*>(glfwGetWindowUserPointer(window));
  module_window->Key(key, scancode, action, mods);
}

void cursor_pos_callback(GLFWwindow* window, double x, double y)
{
  auto module_window = static_cast<GlfwWindowImpl*>(glfwGetWindowUserPointer(window));
  module_window->CursorPos(x, y);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  auto module_window = static_cast<GlfwWindowImpl*>(glfwGetWindowUserPointer(window));
  module_window->Scroll(yoffset);
}
}

GlfwWindow::GlfwWindow()
  : Window()
{
  impl_ = std::make_unique<impl::GlfwWindowImpl>(this);
}

GlfwWindow::~GlfwWindow() = default;

void GlfwWindow::Close()
{
  impl_->Close();
}

bool GlfwWindow::ShouldClose() const
{
  return impl_->ShouldClose();
}

std::vector<std::shared_ptr<Event>> GlfwWindow::PollEvents()
{
  return impl_->PollEvents();
}

void GlfwWindow::SwapBuffers()
{
  impl_->SwapBuffers();
}
}
}
