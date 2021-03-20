#include <twopi/ovr/ovr_swap_chain_gl.h>

#include <assert.h>

#include <iostream>

namespace twopi
{
namespace ovr
{
namespace impl
{
class SwapChainGlImpl
{
public:
  SwapChainGlImpl(ovrSession session, int width, int height)
    : session_(session)
    , width_(width)
    , height_(height)
  {
    ovrTextureSwapChainDesc desc = {};
    
    // Color texture
    desc.Type = ovrTexture_2D;
    desc.ArraySize = 1;
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
    desc.SampleCount = 1;
    desc.StaticImage = ovrFalse;

    ovrResult result = ovr_CreateTextureSwapChainGL(session, &desc, &color_texture_chain_);

    if (OVR_SUCCESS(result))
    {
      int length = 0;
      ovr_GetTextureSwapChainLength(session, color_texture_chain_, &length);

      for (int i = 0; i < length; i++)
      {
        GLuint chain_tex_id;
        result = ovr_GetTextureSwapChainBufferGL(session, color_texture_chain_, i, &chain_tex_id);
        if (!OVR_SUCCESS(result))
          std::cerr << "Failed to get ovr_GetTextureSwapChainBufferGL" << std::endl;

        glBindTexture(GL_TEXTURE_2D, chain_tex_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      }
    }
    else
      std::cerr << "Failed to get ovr_GetTextureSwapChainBufferGL, OVR_FORMAT_R8G8B8A8_UNORM_SRGB" << std::endl;

    // Depth texture
    desc.Format = OVR_FORMAT_D32_FLOAT;

    result = ovr_CreateTextureSwapChainGL(session, &desc, &depth_texture_chain_);

    if (OVR_SUCCESS(result))
    {
      int length = 0;
      ovr_GetTextureSwapChainLength(session, depth_texture_chain_, &length);

      for (int i = 0; i < length; i++)
      {
        GLuint chain_tex_id;
        result = ovr_GetTextureSwapChainBufferGL(session, depth_texture_chain_, i, &chain_tex_id);
        if (!OVR_SUCCESS(result))
          std::cerr << "Failed to get ovr_GetTextureSwapChainBufferGL" << std::endl;

        glBindTexture(GL_TEXTURE_2D, chain_tex_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      }
    }
    else
      std::cerr << "Failed to get ovr_GetTextureSwapChainBufferGL, OVR_FORMAT_D32_FLOAT" << std::endl;

    glGenFramebuffers(1, &fbo_);
  }

  ~SwapChainGlImpl()
  {
    if (color_texture_chain_)
    {
      std::cout << "destroy color swap chain " << color_texture_chain_ << std::endl;
      ovr_DestroyTextureSwapChain(session_, color_texture_chain_);
      color_texture_chain_ = nullptr;
    }
    if (depth_texture_chain_)
    {
      std::cout << "destroy depth swap chain " << depth_texture_chain_ << std::endl;
      ovr_DestroyTextureSwapChain(session_, depth_texture_chain_);
      depth_texture_chain_ = nullptr;
    }
    if (fbo_)
    {
      glDeleteFramebuffers(1, &fbo_);
      fbo_ = 0;
    }
  }

  ovrTextureSwapChain ColorTextureChain()
  {
    return color_texture_chain_;
  }

  ovrTextureSwapChain DepthTextureChain()
  {
    return depth_texture_chain_;
  }

  ovrSizei Size()
  {
    return ovrSizei{ width_, height_ };
  }

  void SetAndClearRenderSurface()
  {
    GLuint cur_color_tex;
    GLuint cur_depth_tex;
    {
      int cur_index;
      ovr_GetTextureSwapChainCurrentIndex(session_, color_texture_chain_, &cur_index);
      ovr_GetTextureSwapChainBufferGL(session_, color_texture_chain_, cur_index, &cur_color_tex);
    }
    {
      int cur_index;
      ovr_GetTextureSwapChainCurrentIndex(session_, depth_texture_chain_, &cur_index);
      ovr_GetTextureSwapChainBufferGL(session_, depth_texture_chain_, cur_index, &cur_depth_tex);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cur_color_tex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, cur_depth_tex, 0);

    glViewport(0, 0, width_, height_);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_FRAMEBUFFER_SRGB);
  }

  void UnsetRenderSurface()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
  }

  void Commit()
  {
    ovr_CommitTextureSwapChain(session_, color_texture_chain_);
    ovr_CommitTextureSwapChain(session_, depth_texture_chain_);
  }

private:
  ovrSession session_ = nullptr;

  int width_ = 0;
  int height_ = 0;
  ovrTextureSwapChain color_texture_chain_ = nullptr;
  ovrTextureSwapChain depth_texture_chain_ = nullptr;

  GLuint fbo_ = 0;
};
}

SwapChainGl::SwapChainGl(ovrSession session, int width, int height)
{
  pimpl_ = std::make_unique<impl::SwapChainGlImpl>(session, width, height);
}

SwapChainGl::~SwapChainGl() = default;

ovrTextureSwapChain SwapChainGl::ColorTextureChain() const
{
  return pimpl_->ColorTextureChain();
}

ovrTextureSwapChain SwapChainGl::DepthTextureChain() const
{
  return pimpl_->DepthTextureChain();
}

ovrSizei SwapChainGl::Size() const
{
  return pimpl_->Size();
}

void SwapChainGl::SetAndClearRenderSurface()
{
  pimpl_->SetAndClearRenderSurface();
}

void SwapChainGl::UnsetRenderSurface()
{
  pimpl_->UnsetRenderSurface();
}

void SwapChainGl::Commit()
{
  pimpl_->Commit();
}
}
}
