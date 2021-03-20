#ifndef TWOPI_APPLICATION_APPLICATION_VK_H_
#define TWOPI_APPLICATION_APPLICATION_VK_H_

#include <twopi/application/application_impl.h>

#include <vector>
#include <string>

#include <Windows.h>

#include <OVR_CAPI_VK.h>

#include <twopi/vk/vulkan_engine.h>

namespace twopi
{
namespace impl
{
class ApplicationVk : public ApplicationImpl
{
public:
  ApplicationVk();
  ~ApplicationVk();

  void Run() override;
  void Stop();

private:
  void InitializeWindow();
  void InitializeOvr();
  void InitializeVulkan();

  void RunWithOvr();
  void RunWithoutOvr();

  void CloseWindow();
  void ShutdownOvr();

  void HandleMessages();

  // OVR Vulkan
  std::vector<std::string> GetOvrInstanceExtensions(ovrGraphicsLuid luid);
  std::vector<std::string> GetOvrDeviceExtensions(ovrGraphicsLuid luid);

  int width_ = 1600;
  int height_ = 900;

  bool running_ = true;

  // Windows
  const LPCWSTR class_name_ = L"TwoPi_Vk";
  HINSTANCE hInstance_ = nullptr;
  HWND hWindow_ = nullptr;

  // Vulkan
  vk::VulkanEngine vulkan_engine_;
};
}
}

#endif // TWOPI_APPLICATION_APPLICATION_VK_H_
