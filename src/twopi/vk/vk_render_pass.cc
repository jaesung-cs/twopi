#include <twopi/vk/vk_render_pass.h>

#include <twopi/vk/vk_device.h>
#include <twopi/vk/vk_image.h>

namespace twopi
{
namespace vkw
{
//
// Creator
//
RenderPass::Creator::Creator(Device device)
  : device_(device)
{
  color_attachment_
    .setSamples(vk::SampleCountFlagBits::e1)
    .setLoadOp(vk::AttachmentLoadOp::eClear)
    .setStoreOp(vk::AttachmentStoreOp::eStore)
    .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
    .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
    .setInitialLayout(vk::ImageLayout::eUndefined)
    .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

  color_attachment_ref_
    .setAttachment(0)
    .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

  subpass_
    .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
    .setColorAttachments(color_attachment_ref_);

  dependency_
    .setSrcSubpass(VK_SUBPASS_EXTERNAL)
    .setDstSubpass(0)
    .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
    .setSrcAccessMask(vk::AccessFlags{})
    .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
    .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
}

RenderPass::Creator::~Creator() = default;

RenderPass::Creator& RenderPass::Creator::SetFormat(Image image)
{
  color_attachment_
    .setFormat(image.Format());

  return *this;
}

RenderPass RenderPass::Creator::Create()
{
  create_info_
    .setAttachments(color_attachment_)
    .setSubpasses(subpass_)
    .setDependencies(dependency_);

  auto handle = device_.createRenderPass(create_info_);
  return RenderPass{ device_, handle };
}

//
// PipelineLayout
//
RenderPass::RenderPass()
{
}

RenderPass::RenderPass(vk::Device device, vk::RenderPass render_pass)
  : device_(device), render_pass_(render_pass)
{
}

RenderPass::~RenderPass() = default;

void RenderPass::Destroy()
{
  device_.destroyRenderPass(render_pass_);
}

RenderPass::operator vk::RenderPass() const
{
  return render_pass_;
}
}
}
