#include <twopi/application/application_gl.h>

namespace twopi
{
namespace impl
{
ApplicationGl::ApplicationGl()
{
}

ApplicationGl::~ApplicationGl()
{
}

void ApplicationGl::Run()
{
}
}
}



// Old implementaion for application with glad and glfw
#if 0
#include <twopi/application/application_gl.h>
#include <twopi/engine.h>

#include <iostream>
#include <stdexcept>
#include <chrono>

#include <windows.h>

#include <glad/glad.h>
#include <glad/glad_wgl.h>

#include <OVR_CAPI_GL.h>

#include <twopi/engine.h>
#include <twopi/ovr/ovr_swap_chain.h>

namespace twopi
{
namespace impl
{
namespace
{
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_KEYDOWN:
    std::cout << static_cast<int>(wParam) << std::endl;
    break;
  case WM_KEYUP:
    std::cout << static_cast<int>(wParam) << std::endl;
    break;
  case WM_DESTROY:
    // TODO: Close requested
    break;
  default:
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
  }
  return 0;
}
}

class ApplicationImpl
{
public:
  ApplicationImpl()
  {
    // Initializes LibOVR, and the Rift
    ovrResult result = ovr_Initialize(nullptr);
    if (!OVR_SUCCESS(result))
      throw std::runtime_error("Failed to initialize libOVR");

    engine_ = std::make_unique<Engine>();
  }

  ~ApplicationImpl()
  {
    if (hWnd_)
    {
      if (hDC_)
      {
        ReleaseDC(hWnd_, hDC_);
        hDC_ = nullptr;
      }
      DestroyWindow(hWnd_);
      hWnd_ = nullptr;
      UnregisterClassW(L"Application", hInstance_);
    }

    // TODO: Why does shutdown cause error?
    ovr_Shutdown();
  }

  void Run()
  {
    hInstance_ = GetModuleHandleW(NULL);

    WNDCLASSW wc = {};
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance_;
    wc.lpszClassName = L"Application";
    RegisterClassW(&wc);
    
    // adjust the window size and show at InitDevice time
    hWnd_ = CreateWindowW(wc.lpszClassName, L"Application", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, 0, 0, hInstance_, 0);
    if (!hWnd_)
      throw std::runtime_error("Failed to create window.");

    SetWindowLongPtr(hWnd_, 0, LONG_PTR(this));

    hDC_ = GetDC(hWnd_);

    RECT size = { 0, 0, 1600, 900 };
    AdjustWindowRect(&size, WS_OVERLAPPEDWINDOW, false);
    const UINT flags = SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW;
    if (!SetWindowPos(hWnd_, nullptr, 0, 0, size.right - size.left, size.bottom - size.top, flags))
      throw std::runtime_error("Failed to set window pos");

    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARBFunc = nullptr;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARBFunc = nullptr;
    {
      // First create a context for the purpose of getting access to wglChoosePixelFormatARB / wglCreateContextAttribsARB.
      PIXELFORMATDESCRIPTOR pfd;
      memset(&pfd, 0, sizeof(pfd));
      pfd.nSize = sizeof(pfd);
      pfd.nVersion = 1;
      pfd.iPixelType = PFD_TYPE_RGBA;
      pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
      pfd.cColorBits = 32;
      pfd.cDepthBits = 16;
      int pf = ChoosePixelFormat(hDC_, &pfd);
      if (!pf)
        throw std::runtime_error("Failed to choose pixel format.");

      if (!SetPixelFormat(hDC_, pf, &pfd))
        throw std::runtime_error("Failed to set pixel format.");

      HGLRC context = wglCreateContext(hDC_);
      if (!context)
        throw std::runtime_error("wglCreateContextfailed.");
      if (!wglMakeCurrent(hDC_, context))
        throw std::runtime_error("wglMakeCurrent failed.");

      wglChoosePixelFormatARBFunc = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
      wglCreateContextAttribsARBFunc = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
      if (!(wglChoosePixelFormatARBFunc && wglCreateContextAttribsARBFunc))
        throw std::runtime_error("Failed to load wglChoosePixelFormatARB and/or wglCreateContextAttribsARB.");

      wglDeleteContext(context);
    }

    // Now create the real context that we will be using.
    int iAttributes[] =
    {
      // WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
      WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
      WGL_COLOR_BITS_ARB, 32,
      WGL_DEPTH_BITS_ARB, 16,
      WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
      WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
      0, 0
    };

    PIXELFORMATDESCRIPTOR pfd = {};
    int pf = 0;
    if (!SetPixelFormat(hDC_, pf, &pfd))
      throw std::runtime_error("SetPixelFormat failed.");

    GLint attribs[16];
    int   attribCount = 0;
    if (debug_)
    {
      attribs[attribCount++] = WGL_CONTEXT_FLAGS_ARB;
      attribs[attribCount++] = WGL_CONTEXT_DEBUG_BIT_ARB;
    }

    attribs[attribCount] = 0;

    wglContext_ = wglCreateContextAttribsARBFunc(hDC_, 0, attribs);
    if (!wglMakeCurrent(hDC_, wglContext_))
      throw std::runtime_error("wglMakeCurrent failed.");

    gladLoadGL();

    // TODO: glad
    /*
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
      throw std::runtime_error("Failed to initialize GLAD");
    */

    glClearColor(0.0f, 1.0f, 0.0f, 1.f);
    glEnable(GL_DEPTH_TEST);

    using namespace std::chrono_literals;
    using Timestamp = std::chrono::steady_clock::time_point;

    constexpr auto oculus_availability_rquest_delay = 1s;
    Timestamp oculus_availablity_request_time = std::chrono::high_resolution_clock::now() - oculus_availability_rquest_delay;

    while (!ShouldCloseApplication())
    {
      oculus_available_ = false;

      Timestamp now = std::chrono::high_resolution_clock::now();

      if (!IsOculusAvailable() && now - oculus_availablity_request_time >= oculus_availability_rquest_delay)
      {
        oculus_availablity_request_time = now;

        ovrGraphicsLuid luid;
        ovrResult result = ovr_Create(&session_, &luid);

        if (!OVR_SUCCESS(result))
          std::cout << "Can't connect to oculus" << std::endl;
        else
          oculus_available_ = true;
      }

      if (IsOculusAvailable())
      {
        std::cout << "Oculus available" << std::endl;

        uint64_t frame_index = 0;

        ovrHmdDesc hmd_desc = ovr_GetHmdDesc(session_);

        ovrSizei window_size = { hmd_desc.Resolution.w / 2, hmd_desc.Resolution.h / 2 };
        // TODO: Set window size
        // glfwSetWindowSize(window_, window_size.w, window_size.h);

        std::shared_ptr<ovr::SwapChainGl> eye_render_textures[2];

        // Make eye render buffers
        for (int eye = 0; eye < 2; eye++)
        {
          ovrSizei ideal_texture_size = ovr_GetFovTextureSize(session_, ovrEyeType(eye), hmd_desc.DefaultEyeFov[eye], 1);
          eye_render_textures[eye] = std::make_shared<ovr::SwapChainGl>(session_, ideal_texture_size.w, ideal_texture_size.h);

          if (eye_render_textures[eye]->ColorTextureChain() == nullptr || eye_render_textures[eye]->DepthTextureChain() == nullptr)
            throw std::runtime_error("Failed to create texture.");
        }

        // Mirror texture
        ovrMirrorTextureDesc desc;
        memset(&desc, 0, sizeof(desc));
        desc.Width = window_size.w;
        desc.Height = window_size.h;
        desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;

        // Create mirror texture and an FBO used to copy mirror texture to back buffer
        ovrMirrorTexture ovr_mirror_texture = nullptr;
        ovrResult result = ovr_CreateMirrorTextureWithOptionsGL(session_, &desc, &ovr_mirror_texture);
        if (!OVR_SUCCESS(result))
        {
          std::cout << "Failed to create mirror texture." << std::endl;
          continue;
        }

        // Configure the mirror read buffer
        GLuint mirror_texture_gl = 0;
        result = ovr_GetMirrorTextureBufferGL(session_, ovr_mirror_texture, &mirror_texture_gl);
        if (!OVR_SUCCESS(result))
          std::cout << "Failed to create mirror texture" << std::endl;

        GLuint mirror_fbo = 0;
        glGenFramebuffers(1, &mirror_fbo);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, mirror_fbo);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirror_texture_gl, 0);
        glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

        // Turn off vsync to let the compositor do its magic
        
        // FloorLevel will give tracking poses where the floor height is 0
        result = ovr_SetTrackingOriginType(session_, ovrTrackingOrigin_FloorLevel);
        if (!OVR_SUCCESS(result))
          std::cout << "Failed to set tracking origin type" << std::endl;

        while (IsOculusAvailable() && !ShouldCloseApplication())
        {
          // TODO: Handle events

          ovrSessionStatus session_status;
          result = ovr_GetSessionStatus(session_, &session_status);
          if (!OVR_SUCCESS(result))
            std::cout << "Failed to get session status" << std::endl;

          if (session_status.ShouldQuit)
          {
            // Because the application is requested to quit, should not request retry
            break;
          }

          if (session_status.IsVisible)
          {
            // Call ovr_GetRenderDesc each frame to get the ovrEyeRenderDesc, as the returned values (e.g. HmdToEyePose) may change at runtime.
            ovrEyeRenderDesc eye_render_descs[2];
            eye_render_descs[0] = ovr_GetRenderDesc(session_, ovrEye_Left, hmd_desc.DefaultEyeFov[0]);
            eye_render_descs[1] = ovr_GetRenderDesc(session_, ovrEye_Right, hmd_desc.DefaultEyeFov[1]);

            // Get eye poses, feeding in correct IPD offset
            ovrPosef eye_render_poses[2];
            ovrPosef hmd_to_eye_poses[2] = { eye_render_descs[0].HmdToEyePose, eye_render_descs[1].HmdToEyePose };

            double sensor_sample_time;    // sensorSampleTime is fed into the layer later
            ovr_GetEyePoses(session_, frame_index, ovrTrue, hmd_to_eye_poses, eye_render_poses, &sensor_sample_time);
            ovrTimewarpProjectionDesc pos_timewarp_projection_desc = {};

            // Render Scene to Eye Buffers
            for (int eye = 0; eye < 2; eye++)
            {
              // Switch to eye render target
              eye_render_textures[eye]->SetAndClearRenderSurface();

              // TODO: Render scene

              // Avoids an error when calling SetAndClearRenderSurface during next iteration.
              // Without this, during the next while loop iteration SetAndClearRenderSurface
              // would bind a framebuffer with an invalid COLOR_ATTACHMENT0 because the texture ID
              // associated with COLOR_ATTACHMENT0 had been unlocked by calling wglDXUnlockObjectsNV.
              eye_render_textures[eye]->UnsetRenderSurface();

              // Commit changes to the textures so they get picked up frame
              eye_render_textures[eye]->Commit();
            }

            // Do distortion rendering, Present and flush/sync
            ovrLayerEyeFovDepth ld = {};
            ld.Header.Type = ovrLayerType_EyeFovDepth;
            ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.
            ld.ProjectionDesc = pos_timewarp_projection_desc;
            ld.SensorSampleTime = sensor_sample_time;

            for (int eye = 0; eye < 2; eye++)
            {
              ld.ColorTexture[eye] = eye_render_textures[eye]->ColorTextureChain();
              ld.DepthTexture[eye] = eye_render_textures[eye]->DepthTextureChain();
              ld.Viewport[eye].Pos.x = 0;
              ld.Viewport[eye].Pos.y = 0;
              ld.Viewport[eye].Size = eye_render_textures[eye]->Size();
              ld.Fov[eye] = hmd_desc.DefaultEyeFov[eye];
              ld.RenderPose[eye] = eye_render_poses[eye];
            }

            ovrLayerHeader* layers = &ld.Header;
            result = ovr_SubmitFrame(session_, frame_index, nullptr, &layers, 1);
            // exit the rendering loop if submit returns an error, will retry on ovrError_DisplayLost
            if (!OVR_SUCCESS(result))
            {
              std::cerr << "Failed to submit frame" << std::endl;
              continue;
            }

            frame_index++;

            // Blit mirror texture to back buffer
            glBindFramebuffer(GL_READ_FRAMEBUFFER, mirror_fbo);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            GLint w = window_size.w;
            GLint h = window_size.h;
            glBlitFramebuffer(
              0, h, w, 0,
              0, 0, w, h,
              GL_COLOR_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
          }

          else
          {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            std::cout << "Oculus session not visible" << std::endl;
          }

          // TODO: Swap buffers
        }

        // Cleanup this session
        if (mirror_fbo)
          glDeleteFramebuffers(1, &mirror_fbo);
        if (ovr_mirror_texture)
          ovr_DestroyMirrorTexture(session_, ovr_mirror_texture);

        for (int eye = 0; eye < 2; eye++)
          eye_render_textures[eye] = nullptr;

        ovr_Destroy(session_);
        std::cout << "session destroyed: " << session_ << std::endl;
        session_ = nullptr;

        // Turn on vsync with glfw
        if (!ShouldCloseApplication())
        {
          // TODO
        }

        break;
      }

      else
      {
        // TODO: Handle events

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // TODO: Swap buffers
      }
    }
  }

private:
  bool ShouldCloseApplication() const
  {
    // TODO
    return false;
  }

  bool IsOculusAvailable() const
  {
    return oculus_available_;
  }

  // Debug
#ifdef _DEBUG
  static constexpr bool debug_ = true;
#else
  static constexpr bool debug_ = false;
#endif

  // Window
  HWND hWnd_ = nullptr;
  HDC hDC_ = nullptr;
  HINSTANCE hInstance_ = nullptr;
  HGLRC wglContext_ = nullptr;
  int width_ = 1600;
  int height_ = 900;

  // Oculus
  bool oculus_available_ = false;
  ovrSession session_ = nullptr;

  std::unique_ptr<Engine> engine_;
};
}

Application::Application()
{
  pimpl_ = std::make_unique<impl::ApplicationImpl>();
}

Application::~Application() = default;

void Application::Run()
{
  pimpl_->Run();
}
}
#endif
