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

  depth_attachment_
    .setFormat(vk::Format::eD24UnormS8Uint)
    .setSamples(vk::SampleCountFlagBits::e1)
    .setLoadOp(vk::AttachmentLoadOp::eClear)
    .setStoreOp(vk::AttachmentStoreOp::eDontCare)
    .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
    .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
    .setInitialLayout(vk::ImageLayout::eUndefined)
    .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

  depth_attachment_ref_
    .setAttachment(1)
    .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

  subpass_
    .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
    .setColorAttachments(color_attachment_ref_)
    .setPDepthStencilAttachment(&depth_attachment_ref_);

  dependency_
    .setSrcSubpass(VK_SUBPASS_EXTERNAL)
    .setDstSubpass(0)
    .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
    .setSrcAccessMask(vk::AccessFlags{})
    .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
    .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);
}

RenderPass::Creator::~Creator() = default;

RenderPass::Creator& RenderPass::Creator::SetFormat(Image image)
{
  format_ = image.Format();

  return *this;
}

RenderPass::Creator& RenderPass::Creator::SetMultisample4()
{
  color_attachment_
    .setSamples(vk::SampleCountFlagBits::e4)
    .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

  depth_attachment_.setSamples(vk::SampleCountFlagBits::e4);

  has_color_attachment_resolve_ = true;
  color_attachment_resolve_
    .setSamples(vk::SampleCountFlagBits::e1)
    .setLoadOp(vk::AttachmentLoadOp::eDontCare)
    .setStoreOp(vk::AttachmentStoreOp::eStore)
    .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
    .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
    .setInitialLayout(vk::ImageLayout::eUndefined)
    .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

  color_attachment_resolve_ref_
    .setAttachment(2)
    .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

  subpass_
    .setResolveAttachments(color_attachment_resolve_ref_);

  return *this;
}

RenderPass RenderPass::Creator::Create()
{
  color_attachment_
    .setFormat(format_);

  color_attachment_resolve_
    .setFormat(format_);

  std::vector<vk::AttachmentDescription> attachments = { color_attachment_, depth_attachment_, color_attachment_resolve_ };
  create_info_
    .setAttachments(attachments)
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
