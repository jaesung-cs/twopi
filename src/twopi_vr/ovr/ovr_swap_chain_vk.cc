#include <twopi/ovr/ovr_swap_chain_vk.h>

#include <assert.h>

#include <iostream>

namespace twopi
{
namespace ovr
{
SwapChainVk::SwapChainVk(ovrSession session, int width, int height)
  : session_(session)
  , width_(width)
  , height_(height)
{
}

SwapChainVk::~SwapChainVk()
{
}

ovrTextureSwapChain SwapChainVk::ColorTextureChain() const
{
  return nullptr;
}

ovrTextureSwapChain SwapChainVk::DepthTextureChain() const
{
  return nullptr;
}

ovrSizei SwapChainVk::Size() const
{
  return ovrSizei{ width_, height_ };
}

void SwapChainVk::SetAndClearRenderSurface()
{
}

void SwapChainVk::UnsetRenderSurface()
{
}

void SwapChainVk::Commit()
{
}
}
}
