#include <twopi/application/application.h>

#include <chrono>
#include <thread>
#include <iostream>

#include <twopi/window/glfw_window.h>
#include <twopi/window/event/mouse_button_event.h>
#include <twopi/window/event/mouse_move_event.h>
#include <twopi/window/event/mouse_wheel_event.h>
#include <twopi/window/event/keyboard_event.h>
#include <twopi/scene/camera.h>
#include <twopi/scene/camera_orbit_control.h>
#include <twopi/gl/gl_engine.h>

namespace twopi
{
namespace app
{
namespace impl
{
class ApplicationImpl
{
public:
  ApplicationImpl()
  {
    window_ = std::make_shared<window::GlfwWindow>();

    gl_engine_ = std::make_shared<gl::Engine>();
    gl_engine_->SetViewport(0, 0, window_->Width(), window_->Height());

    camera_ = std::make_shared<scene::Camera>();
    camera_->SetScreenSize(window_->Width(), window_->Height());

    camera_control_ = std::make_shared<scene::CameraOrbitControl>();

    camera_control_->SetCamera(camera_);
  }

  ~ApplicationImpl() = default;

  void Run()
  {
    while (!ShouldTerminate())
    {
      PollEvents();

      gl_engine_->UpdateCamera(camera_);
      gl_engine_->Draw();

      SwapBuffers();
    }
  }

private:
  void PollEvents()
  {
    const auto events = window_->PollEvents();

    for (const auto event : events)
    {
      if (auto mouse_button_event = std::dynamic_pointer_cast<window::MouseButtonEvent>(event))
      {
        mouse_last_x_ = mouse_button_event->X();
        mouse_last_y_ = mouse_button_event->Y();

        int mouse_button_index = -1;
        switch (mouse_button_event->Button())
        {
        case window::MouseButton::LEFT: mouse_button_index = 0; break;
        case window::MouseButton::RIGHT: mouse_button_index = 1; break;
        case window::MouseButton::MIDDLE: mouse_button_index = 2; break;
        }

        int mouse_button_state_index = -1;
        switch (mouse_button_event->State())
        {
        case window::MouseButtonState::RELEASED: mouse_button_state_index = 0; break;
        case window::MouseButtonState::PRESSED: mouse_button_state_index = 1; break;
        }

        if (mouse_button_index >= 0)
          mouse_buttons_[mouse_button_index] = mouse_button_state_index;
      }

      else if (auto mouse_move_event = std::dynamic_pointer_cast<window::MouseMoveEvent>(event))
      {
        const auto x = mouse_move_event->X();
        const auto y = mouse_move_event->Y();

        const auto dx = x - mouse_last_x_;
        const auto dy = y - mouse_last_y_;

        if (mouse_buttons_[0] && mouse_buttons_[1])
          camera_control_->ZoomByPixels(dx, dy);

        else if (mouse_buttons_[0])
          camera_control_->RotateByPixels(dx, dy);

        else if (mouse_buttons_[1])
          camera_control_->TranslateByPixels(dx, dy);

        mouse_last_x_ = x;
        mouse_last_y_ = y;
      }

      else if (auto mouse_wheel_event = std::dynamic_pointer_cast<window::MouseWheelEvent>(event))
      {
        const auto scroll = mouse_wheel_event->Scroll();
        camera_control_->ZoomByWheel(scroll);
      }

      else if (auto keyboard_event = std::dynamic_pointer_cast<window::KeyboardEvent>(event))
      {
        if (keyboard_event->Key() == '`' && keyboard_event->State() == window::KeyState::PRESSED)
          window_->Close();
      }
    }

    camera_control_->Update();
  }

  void SwapBuffers()
  {
    window_->SwapBuffers();
  }

  bool ShouldTerminate()
  {
    return window_->ShouldClose();
  }

  const double frames_per_second_ = 60.;
  std::shared_ptr<window::Window> window_;
  std::shared_ptr<gl::Engine> gl_engine_;

  // Mouse
  int mouse_last_x_ = 0;
  int mouse_last_y_ = 0;
  int mouse_buttons_[3] = { 0, 0, 0 };

  // Camera
  std::shared_ptr<scene::Camera> camera_;
  std::shared_ptr<scene::CameraOrbitControl> camera_control_;
};
}

Application::Application()
{
  impl_ = std::make_unique<impl::ApplicationImpl>();
}

Application::~Application() = default;

void Application::Run()
{
  impl_->Run();
}
}
}
