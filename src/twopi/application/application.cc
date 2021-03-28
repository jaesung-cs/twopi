#include <twopi/application/application.h>

#include <chrono>
#include <thread>
#include <iostream>

#include <twopi/window/glfw_window.h>
#include <twopi/window/event/mouse_button_event.h>
#include <twopi/window/event/mouse_move_event.h>
#include <twopi/window/event/mouse_wheel_event.h>
#include <twopi/window/event/keyboard_event.h>
#include <twopi/window/event/resize_event.h>
#include <twopi/scene/camera.h>
#include <twopi/scene/vr_camera.h>
#include <twopi/scene/camera_orbit_control.h>
#include <twopi/scene/light.h>
#include <twopi/vk/vk_engine.h>

#include <glm/glm.hpp>

namespace twopi
{
namespace app
{
class Application::Impl
{
public:
  Impl()
  {
    std::memset(key_pressed_, false, sizeof key_pressed_);

    window_ = std::make_shared<window::GlfwWindow>();

    vk_engine_ = std::make_shared<vkw::Engine>(window_);

    camera_ = std::make_shared<scene::Camera>();
    camera_->SetScreenSize(window_->Width(), window_->Height());

    vr_camera_ = std::make_shared<scene::VrCamera>();
    vr_camera_->SetScreenSize(window_->Width(), window_->Height());

    camera_control_ = std::make_shared<scene::CameraOrbitControl>();

    current_camera_ = camera_;
    camera_control_->SetCamera(current_camera_);

    auto light = std::make_shared<scene::Light>();
    light->SetPosition(glm::vec3{ 0.f, 0.f, 1.f });
    light->SetAmbient(glm::vec3{ 0.2f, 0.2f, 0.2f });
    light->SetDiffuse(glm::vec3{ 0.8f, 0.8f, 0.8f });
    light->SetSpecular(glm::vec3{ 1.f, 1.f, 1.f });
    lights_.emplace_back(std::move(light));
  }

  ~Impl() = default;

  void Run()
  {
    core::Timestamp previous_timestamp = core::Clock::now();

    while (!ShouldTerminate())
    {
      const auto current_timestamp = core::Clock::now();

      // User input
      PollEvents(current_timestamp);

      // Keyboard motion
      const auto dt = std::chrono::duration<double>(current_timestamp - previous_timestamp).count();
      UpdateKeyboard(dt);

      // Light position updated to camera eye
      lights_[0]->SetPosition(current_camera_->Eye() - current_camera_->Center());

      // Draw on Vulkan surface
      vk_engine_->UpdateCamera(current_camera_);
      vk_engine_->Draw();

      SwapBuffers();

      previous_timestamp = current_timestamp;
    }
  }

private:
  void PollEvents(core::Timestamp timestamp)
  {
    const auto events = window_->PollEvents(timestamp);

    bool resized = false;
    int resize_width = 1600;
    int resize_height = 900;

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

        else
        {
          switch (keyboard_event->State())
          {
          case window::KeyState::PRESSED:
            key_pressed_[keyboard_event->Key()] = true;
            break;
          case window::KeyState::RELEASED:
            key_pressed_[keyboard_event->Key()] = false;
            break;
          }

          if (keyboard_event->State() == window::KeyState::PRESSED)
          {
            if (keyboard_event->Key() == '1')
            {
              current_camera_ = camera_;
              camera_control_->SetCamera(current_camera_);
            }
            else if (keyboard_event->Key() == '2')
            {
              current_camera_ = vr_camera_;
              camera_control_->SetCamera(current_camera_);
            }
          }
        }
      }

      else if (auto resize_event = std::dynamic_pointer_cast<window::ResizeEvent>(event))
      {
        resized = true;
        resize_width = resize_event->Width();
        resize_height = resize_event->Height();
      }
    }

    if (resized)
    {
      camera_->SetScreenSize(resize_width, resize_height);
      vr_camera_->SetScreenSize(resize_width, resize_height);

      // Update framebuffer size
      vk_engine_->Resize(resize_width, resize_height);
    }

    camera_control_->Update();
  }

  void UpdateKeyboard(double dt)
  {
    // Move camera
    if (key_pressed_['W'])
      camera_control_->MoveForward(dt);
    if (key_pressed_['S'])
      camera_control_->MoveForward(-dt);
    if (key_pressed_['A'])
      camera_control_->MoveRight(-dt);
    if (key_pressed_['D'])
      camera_control_->MoveRight(dt);
    if (key_pressed_[' '])
      camera_control_->MoveUp(dt);
  }

  void SwapBuffers()
  {
    // Vulkan engine manages buffering
    // window_->SwapBuffers();
  }

  bool ShouldTerminate()
  {
    return window_->ShouldClose();
  }

  const double frames_per_second_ = 60.;
  std::shared_ptr<window::Window> window_;

  // Vulkan engine
  std::shared_ptr<vkw::Engine> vk_engine_;

  // Mouse
  int mouse_last_x_ = 0;
  int mouse_last_y_ = 0;
  int mouse_buttons_[3] = { 0, 0, 0 };

  // Keyboard
  bool key_pressed_[512];

  // Camera
  std::shared_ptr<scene::Camera> camera_;
  std::shared_ptr<scene::VrCamera> vr_camera_;
  std::shared_ptr<scene::Camera> current_camera_;
  std::shared_ptr<scene::CameraOrbitControl> camera_control_;

  // Light
  std::vector<std::shared_ptr<scene::Light>> lights_;
};

Application::Application()
{
  impl_ = std::make_unique<Impl>();
}

Application::~Application() = default;

void Application::Run()
{
  impl_->Run();
}
}
}
