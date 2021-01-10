#ifndef TWOPI_OVR_OVR_SWAP_CHAIN_GL_H_
#define TWOPI_OVR_OVR_SWAP_CHAIN_GL_H_

#include <memory>

#include <glad/glad.h>

#include <OVR_CAPI_GL.h>

namespace twopi
{
namespace ovr
{
namespace impl
{
class SwapChainGlImpl;
}

class SwapChainGl
{
public:
  SwapChainGl() = delete;
  SwapChainGl(ovrSession session, int width, int height);
  ~SwapChainGl();

  ovrTextureSwapChain ColorTextureChain() const;
  ovrTextureSwapChain DepthTextureChain() const;
  ovrSizei Size() const;

  void SetAndClearRenderSurface();
  void UnsetRenderSurface();
  void Commit();

private:
  std::unique_ptr<impl::SwapChainGlImpl> pimpl_;
};
}
}

#endif // TWOPI_OVR_OVR_SWAP_CHAIN_GL_H_
