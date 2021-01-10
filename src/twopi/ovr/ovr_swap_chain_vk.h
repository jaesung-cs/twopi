#ifndef TWOPI_OVR_OVR_SWAP_CHAIN_VK_H_
#define TWOPI_OVR_OVR_SWAP_CHAIN_VK_H_

#include <memory>

#include <vulkan/vulkan.h>

#include <OVR_CAPI_VK.h>
#include <OVR_CAPI.h>

namespace twopi
{
namespace ovr
{
class SwapChainVk
{
public:
  SwapChainVk() = delete;
  SwapChainVk(ovrSession session, int width, int height);
  ~SwapChainVk();

  ovrTextureSwapChain ColorTextureChain() const;
  ovrTextureSwapChain DepthTextureChain() const;
  ovrSizei Size() const;

  void SetAndClearRenderSurface();
  void UnsetRenderSurface();
  void Commit();

private:
  ovrSession session_;
  int width_;
  int height_;
};
}
}

#endif // TWOPI_OVR_OVR_SWAP_CHAIN_VK_H_
