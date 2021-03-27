#include <twopi/window/glfw_window.h>

#include <memory>
#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <twopi/window/window.h>
#include <twopi/window/event/types.h>
#include <twopi/window/event/keyboard_event.h>
#include <twopi/window/event/mouse_button_event.h>
#include <twopi/window/event/mouse_move_event.h>
#include <twopi/window/event/mouse_wheel_event.h>
#include <twopi/window/event/resize_event.h>
#include <twopi/core/error.h>

namespace twopi
{
namespace window
{
class GlfwWindow::Impl
{
private:
  static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
  {
    auto module_window = static_cast<Impl*>(glfwGetWindowUserPointer(window));
    module_window->MouseButton(button, action, mods);
  }

  static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
  {
    auto module_window = static_cast<Impl*>(glfwGetWindowUserPointer(window));
    module_window->Key(key, scancode, action, mods);
  }

  static void cursor_pos_callback(GLFWwindow* window, double x, double y)
  {
    auto module_window = static_cast<Impl*>(glfwGetWindowUserPointer(window));
    module_window->CursorPos(x, y);
  }

  static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
  {
    auto module_window = static_cast<Impl*>(glfwGetWindowUserPointer(window));
    module_window->Scroll(yoffset);
  }

  static void resize_callback(GLFWwindow* window, int width, int height)
  {
    auto module_window = static_cast<Impl*>(glfwGetWindowUserPointer(window));
    module_window->Resize(width, height);
  }

public:
  Impl() = delete;

  Impl(GlfwWindow* base)
    : base_(base)
  {
    if (!glfwInit())
    {
      throw core::Error("Failed to initialize GLFW.");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window_ = glfwCreateWindow(base_->Width(), base_->Height(), "Twopi", NULL, NULL);

    glfwSetWindowUserPointer(window_, this);
    glfwSetMouseButtonCallback(window_, mouse_button_callback);
    glfwSetCursorPosCallback(window_, cursor_pos_callback);
    glfwSetKeyCallback(window_, key_callback);
    glfwSetScrollCallback(window_, scroll_callback);
    glfwSetWindowSizeCallback(window_, resize_callback);

    glfwSetWindowPos(window_, 100, 100);

    glfwMakeContextCurrent(window_);
  }

  ~Impl()
  {
    glfwDestroyWindow(window_);
    glfwTerminate();
  }

  GLFWwindow* Handle() const
  {
    return window_;
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

    return std::move(event_queue_);
  }

  std::vector<std::shared_ptr<Event>> PollEvents(core::Timestamp timestamp)
  {
    const auto event_queue = PollEvents();
    for (const auto event : event_queue)
      event->SetTimestamp(timestamp);

    return event_queue;
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

    case GLFW_REPEAT:
      event_action = KeyState::REPEAT;
      break;

    default:
      event_action = KeyState::UNDEFINED;
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

    const auto event = std::make_shared<MouseWheelEvent>(scrolli);
    event_queue_.push_back(event);
  }

  void Resize(int width, int height)
  {
    base_->Resized(width, height);

    const auto event = std::make_shared<ResizeEvent>(width, height);
    event_queue_.push_back(event);
  }

private:
  GlfwWindow* base_;
  GLFWwindow* window_;

  std::vector<std::shared_ptr<Event>> event_queue_;

  core::Timestamp timestamp_;
};

GlfwWindow::GlfwWindow()
  : Window()
{
  impl_ = std::make_unique<Impl>(this);
}

GlfwWindow::~GlfwWindow() = default;

GLFWwindow* GlfwWindow::Handle() const
{
  return impl_->Handle();
}

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

std::vector<std::shared_ptr<Event>> GlfwWindow::PollEvents(core::Timestamp timestamp)
{
  return impl_->PollEvents(timestamp);
}

void GlfwWindow::SwapBuffers()
{
  impl_->SwapBuffers();
}
}
}
