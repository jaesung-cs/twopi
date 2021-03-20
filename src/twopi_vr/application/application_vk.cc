#include <twopi/application/application_vk.h>

#include <iostream>
#include <thread>
#include <chrono>

namespace twopi
{
namespace impl
{
namespace
{
LRESULT CALLBACK WindowProc(_In_ HWND hWindow, _In_ UINT msg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  ApplicationVk* app = reinterpret_cast<ApplicationVk*>(GetWindowLongPtr(hWindow, 0));
  switch (msg)
  {
  case WM_KEYDOWN:
    // Left shift
    if (wParam == 16)
    {
    }

    // Left ctrl
    else if (wParam == 17)
    {
    }

    // `
    else if (wParam == 192)
    {
      app->Stop();
    }

    // ESC
    else if (wParam == 27)
    {
      app->Stop();
    }

    break;

  case WM_KEYUP:
    break;

  case WM_SYSKEYDOWN:
    // Alt
    if (wParam == 18)
    {
    }
    break;

  case WM_SYSKEYUP:
    break;

  case WM_DESTROY:
    app->Stop();
    break;

  default:
    return DefWindowProcW(hWindow, msg, wParam, lParam);
  }

  return 0;
}
}

ApplicationVk::ApplicationVk()
{
}

ApplicationVk::~ApplicationVk()
{
}

void ApplicationVk::Stop()
{
  running_ = false;
}

void ApplicationVk::Run()
{
  RunWithoutOvr();
  // RunWithOvr();
}

void ApplicationVk::RunWithoutOvr()
{
  using namespace std::chrono_literals;

  InitializeWindow();
  InitializeVulkan();

  while (true)
  {
    HandleMessages();

    if (!running_)
      break;

    vulkan_engine_.Draw();

    std::this_thread::sleep_for(10ms);
  }

  vulkan_engine_.DeviceWaitIdle();
}

void ApplicationVk::RunWithOvr()
{
  /*
  * OVR Vulkan functions:
  * 
  * Related to device:
  *   ovr_GetInstanceExtensionsVk (o)
  *   ovr_GetDeviceExtensionsVk (o)
  *   ovr_GetSessionPhysicalDeviceVk (o)
  *   ovr_SetSynchronizationQueueVk
  * 
  * Related to swapchain:
  *   ovr_CreateTextureSwapChainVk
  *   ovr_GetTextureSwapChainBufferVk
  *   ovr_CreateMirrorTextureWithOptionsVk
  *   ovr_GetMirrorTextureBufferVk
  */

  using namespace std::chrono_literals;

  InitializeWindow();
  InitializeOvr();
  
  while (true)
  {
    HandleMessages();

    if (!running_)
      break;

    ovrSession session;
    ovrGraphicsLuid luid;

    ovrResult result = ovr_Create(&session, &luid);
    if (!OVR_SUCCESS(result))
    {
      std::cout << "Failed to create ovr session" << std::endl;
      std::this_thread::sleep_for(10ms);
      continue;
    }

    try
    {
      std::cout << "Found ovr session" << std::endl;

      ovrHmdDesc hmd_desc = ovr_GetHmdDesc(session);

      // Setup Window and Graphics
      // Note: the mirror window can be any size, for this sample we use 1/2 the HMD resolution
      ovrSizei ovr_size = { hmd_desc.Resolution.w / 2, hmd_desc.Resolution.h / 2 };

      VkExtent2D size;
      size.width = ovr_size.w;
      size.height = ovr_size.h;
      std::cout << "Viewport: " + std::to_string(size.width) + "x" + std::to_string(size.height) << std::endl;

      auto ovr_instance_extensions = GetOvrInstanceExtensions(luid);
      auto ovr_device_extensions = GetOvrDeviceExtensions(luid);

      std::cout << ovr_instance_extensions.size() << " instance extensions required by ovr" << std::endl;
      for (int i = 0; i < ovr_instance_extensions.size(); i++)
        std::cout << "  " << i << ": " << ovr_instance_extensions[i] << std::endl;

      std::cout << ovr_device_extensions.size() << " device extensions required by ovr" << std::endl;
      for (int i = 0; i < ovr_device_extensions.size(); i++)
        std::cout << "  " << i << ": " << ovr_device_extensions[i] << std::endl;

      // Initialize vulkan engine
      vk::VulkanEngine::PreparationOptions preparation_params;
      preparation_params.hInstance = hInstance_;
      preparation_params.hWindow = hWindow_;

      preparation_params.enable_validation_layers = true;

      preparation_params.instance_extensions = std::move(ovr_instance_extensions);
      preparation_params.device_extensions = std::move(ovr_device_extensions);

      preparation_params.physical_device_selector = [session, luid](VkInstance instance) {
        VkPhysicalDevice physical_device;
        ovr_GetSessionPhysicalDeviceVk(session, luid, instance, &physical_device);
        return physical_device;
      };

      vulkan_engine_.Prepare(preparation_params);

      // Let the compositor know which queue to synchronize with
      ovr_SetSynchronizationQueueVk(session, vulkan_engine_.GraphicsQueue());

      int64_t frame_index = 0;

      while (true)
      {
        HandleMessages();

        if (!running_)
          break;

        /*
        ovr_WaitToBeginFrame(session, frame_index);
        ovr_BeginFrame(session, frame_index);
        */

        // Call ovr_GetRenderDesc each frame to get the ovrEyeRenderDesc, as the returned values (e.g. hmdToEyePose) may change at runtime.
        /*
        std::vector<ovrEyeType> eyes = { ovrEye_Left, ovrEye_Right };
        ovrEyeRenderDesc eye_render_desc[ovrEye_Count];
        for (auto eye : eyes)
          eye_render_desc[eye] = ovr_GetRenderDesc(session, eye, hmd_desc.DefaultEyeFov[eye]);

        // Get eye poses, feeding in correct IPD offset
        ovrPosef hmd_to_eye_pose[ovrEye_Count] = {
          eye_render_desc[ovrEye_Left].HmdToEyePose, eye_render_desc[ovrEye_Right].HmdToEyePose
        };
        ovrPosef eye_render_pose[ovrEye_Count];
        double sensor_sample_time; // sensorSampleTime is fed into ovr_SubmitFrame later
        ovr_GetEyePoses(session, frame_index, ovrTrue, hmd_to_eye_pose, eye_render_pose, &sensor_sample_time);

        // TODO: draw command for each eyes
        ovrTimewarpProjectionDesc posTimewarpProjectionDesc = {};
        */

        // TODO: ovr_CommitTextureSwapChain with ovr color and depth chains

        // TODO: Submit frame
        // Submit rendered eyes as an EyeFovDepth layer
        /*
        ovrLayerEyeFovDepth ld = {};
        ld.Header.Type = ovrLayerType_EyeFovDepth;
        ld.Header.Flags = 0;
        ld.ProjectionDesc = pos_timewarp_projection_desc;
        ld.SensorSampleTime = sensor_sample_time;

        for (auto eye : { ovrEye_Left, ovrEye_Right })
        {
          ld.ColorTexture[eye] = perEye[eye].tex.textureChain;
          ld.DepthTexture[eye] = perEye[eye].tex.depthChain;
          ld.Viewport[eye] = perEye[eye].tex.GetViewport();
          ld.Fov[eye] = hmdDesc.DefaultEyeFov[eye];
          ld.RenderPose[eye] = eyeRenderPose[eye];
        }

        ovrLayerHeader* layers = &ld.Header;
        ovrLayerHeader* layers[2] = { &layer0.Header, &layer1.Header };
        ovr_EndFrame(session, frame_index, nullptr, layers, 2);
        */

        vulkan_engine_.Draw();

        frame_index++;

        std::this_thread::sleep_for(10ms);
      }
    }
    catch (const std::exception& e)
    {
      std::cerr << e.what() << std::endl;
    }

    vulkan_engine_.DeviceWaitIdle();

    ovr_Destroy(session);
  }

  CloseWindow();
  ShutdownOvr();
}

std::vector<std::string> ApplicationVk::GetOvrInstanceExtensions(ovrGraphicsLuid luid)
{
  uint32_t extension_names_size = 4096;
  std::string extension_names(extension_names_size, '\0');
  auto ret = ovr_GetInstanceExtensionsVk(luid, extension_names.data(), &extension_names_size);
  extension_names.resize(static_cast<size_t>(extension_names_size) + 1);

  if (!OVR_SUCCESS(ret))
  {
    ovrErrorInfo info;
    ovr_GetLastErrorInfo(&info);
    throw std::runtime_error(std::string("Error: ovr_GetInstanceExtensionsVk, ") + info.ErrorString);
  }

  std::vector<std::string> extensions;
  std::string current_extension;

  // Parse extension names
  for (int i = 0; i < extension_names.size(); i++)
  {
    const auto& c = extension_names[i];

    if (c == ' ')
    {
      extensions.emplace_back(std::move(current_extension));
      current_extension = "";
    }
    else
      current_extension += c;
  }

  if (!current_extension.empty())
    extensions.emplace_back(std::move(current_extension));

  return extensions;
}

std::vector<std::string> ApplicationVk::GetOvrDeviceExtensions(ovrGraphicsLuid luid)
{
  uint32_t device_extension_names_size = 4096;
  std::string device_extension_names(device_extension_names_size, '\0');
  auto ret = ovr_GetDeviceExtensionsVk(luid, device_extension_names.data(), &device_extension_names_size);
  device_extension_names.resize(static_cast<size_t>(device_extension_names_size) + 1);

  if (!OVR_SUCCESS(ret))
  {
    ovrErrorInfo info;
    ovr_GetLastErrorInfo(&info);
    throw std::runtime_error(std::string("Error: ovr_GetInstanceExtensionsVk, ") + info.ErrorString);
  }

  std::vector<std::string> device_extensions;
  std::string current_extension;

  // Parse extension names
  for (int i = 0; i < device_extension_names.size(); i++)
  {
    const auto& c = device_extension_names[i];

    if (c == ' ')
    {
      device_extensions.emplace_back(std::move(current_extension));
      current_extension = "";
    }
    else
      current_extension += c;
  }

  if (!current_extension.empty())
    device_extensions.emplace_back(std::move(current_extension));

  return device_extensions;
}

void ApplicationVk::InitializeWindow()
{
  hInstance_ = GetModuleHandleW(NULL);

  WNDCLASSW wc{};
  wc.style = CS_CLASSDC;
  wc.lpfnWndProc = WindowProc;
  wc.cbWndExtra = sizeof(this);
  wc.hInstance = hInstance_;
  wc.lpszClassName = class_name_;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  RegisterClassW(&wc);

  // adjust the window size and show at InitDevice time
#if defined(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)
  // Make sure we're 1:1 for HMD pixels
  SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif

  hWindow_ = CreateWindowW(wc.lpszClassName, L"TwoPi Vk", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, 0, 0, hInstance_, 0);
  if (!hWindow_)
    return;

  SetWindowLongPtr(hWindow_, 0, LONG_PTR(this));

  // Show window
  RECT windowSize = { 0, 0, width_, height_ };
  AdjustWindowRect(&windowSize, WS_OVERLAPPEDWINDOW, false);
  const UINT flags = SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW;
  if (!SetWindowPos(hWindow_, nullptr, 0, 0, windowSize.right - windowSize.left, windowSize.bottom - windowSize.top, flags))
    std::cout << "Failed to set window pos" << std::endl;
}

void ApplicationVk::InitializeOvr()
{
  // Initializes LibOVR, and the Rift
  ovrInitParams initParams = { ovrInit_RequestVersion | ovrInit_FocusAware, OVR_MINOR_VERSION, NULL, 0, 0 };
  ovrResult result = ovr_Initialize(&initParams);
  if (!OVR_SUCCESS(result))
  {
    std::cout << "Failed to initialize libOVR." << std::endl;
    return;
  }
}

void ApplicationVk::InitializeVulkan()
{
  vk::VulkanEngine::PreparationOptions preparation_params;
  preparation_params.hInstance = hInstance_;
  preparation_params.hWindow = hWindow_;

  preparation_params.enable_validation_layers = true;

  vulkan_engine_.Prepare(preparation_params);
}

void ApplicationVk::CloseWindow()
{
  if (hWindow_)
  {
    DestroyWindow(hWindow_);
    hWindow_ = nullptr;
    UnregisterClassW(class_name_, hInstance_);
  }
}

void ApplicationVk::ShutdownOvr()
{
  ovr_Shutdown();
}

void ApplicationVk::HandleMessages()
{
  MSG msg;
  while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}
}
}
