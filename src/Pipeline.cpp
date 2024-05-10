#include "../inc/Pipeline.h"

vkInit::PipelineBuilder::PipelineBuilder() {
}

vkInit::PipelineBuilder::~PipelineBuilder() {
  reset();
}

void vkInit::PipelineBuilder::init(vk::Device device) {
  mDevice = device;

  reset();

  configure_input_assembly();
  make_rasterizer_info();
  configure_multisampling();
  configure_color_blending();
  mPipelineInfo.basePipelineHandle = nullptr;
}

void vkInit::PipelineBuilder::reset() {
  mPipelineInfo.flags = vk::PipelineCreateFlags();

  reset_vertex_format();
  reset_shader_modules();
  reset_renderpass_attachments();
  reset_descriptor_set_layout();
}

void vkInit::PipelineBuilder::specify_vertex_format(
  vk::VertexInputBindingDescription bindingDescription,
  std::vector<vk::VertexInputAttributeDescription> attributeDescriptions
) {
  mBindingDescription = bindingDescription;
  mAttributeDescriptions = attributeDescriptions;

  mVertexInputInfo.vertexBindingDescriptionCount   = 1;
  mVertexInputInfo.pVertexBindingDescriptions      = &mBindingDescription;
  mVertexInputInfo.vertexAttributeDescriptionCount =
    static_cast<uint32_t>(mAttributeDescriptions.size());
  mVertexInputInfo.pVertexAttributeDescriptions    =
    mAttributeDescriptions.data();
}

void vkInit::PipelineBuilder::specify_vertex_shader(const char* filename) {
  if (mVertexShader) {
    mDevice.destroyShaderModule(mVertexShader);
    mVertexShader = nullptr;
  }
  mVertexShader = vkUtil::createModule(filename, mDevice, true);
  mVertexShaderInfo = make_shader_info(
    mVertexShader,
    vk::ShaderStageFlagBits::eVertex
  );
  mShaderStages.push_back(mVertexShaderInfo);
}

void vkInit::PipelineBuilder::specify_fragment_shader(const char* filename) {
  if (mFragmentShader) {
    mDevice.destroyShaderModule(mFragmentShader);
    mFragmentShader = nullptr;
  }
  mFragmentShader = vkUtil::createModule(filename, mDevice, true);
  mFragmentShaderInfo = make_shader_info(
    mFragmentShader,
    vk::ShaderStageFlagBits::eFragment
  );
  mShaderStages.push_back(mFragmentShaderInfo);
}

void vkInit::PipelineBuilder::specify_swapchain_extent(
  vk::Extent2D screen_size
) {
  mSwapchainExtent = screen_size;
}

void vkInit::PipelineBuilder::specify_depth_attachment(
  const vk::Format& depthFormat,
  uint32_t attachment_index
) {
  mDepthState.flags                 =
    vk::PipelineDepthStencilStateCreateFlags();
  mDepthState.depthTestEnable       = true;
  mDepthState.depthWriteEnable      = true;
  mDepthState.depthCompareOp        = vk::CompareOp::eLess;
  mDepthState.depthBoundsTestEnable = false;
  mDepthState.stencilTestEnable     = false;

  mPipelineInfo.pDepthStencilState = &mDepthState;
  mAttachmentDescriptions.insert({
    attachment_index,
    make_renderpass_attachment(
      depthFormat,
      vk::AttachmentLoadOp::eClear,
      vk::AttachmentStoreOp::eDontCare,
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eDepthStencilAttachmentOptimal
    )
  });
  mAttachmentReferences.insert({
    attachment_index,
    make_attachment_reference(
      attachment_index,
      vk::ImageLayout::eDepthStencilAttachmentOptimal
    )
  });
}

void vkInit::PipelineBuilder::clear_depth_attachment() {
  mPipelineInfo.pDepthStencilState = nullptr;
}

void vkInit::PipelineBuilder::add_color_attachment(
  const vk::Format& format,
  uint32_t attachment_index
) {
  vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eDontCare;
  if (mOverwrite) {
    loadOp = vk::AttachmentLoadOp::eLoad;
  }

  vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore;

  vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined;
  if (mOverwrite) {
    initialLayout = vk::ImageLayout::ePresentSrcKHR;
  }

  vk::ImageLayout finalLayout = vk::ImageLayout::ePresentSrcKHR;

  mAttachmentDescriptions.insert({
    attachment_index,
    make_renderpass_attachment(
      format,
      loadOp,
      storeOp,
      initialLayout,
      finalLayout
    )
  });

  mAttachmentReferences.insert({
    attachment_index,
    make_attachment_reference(
      attachment_index,
      vk::ImageLayout::eColorAttachmentOptimal
    )
  });
}

void vkInit::PipelineBuilder::set_overwrite_mode(bool mode) {
  mOverwrite = mode;
}

vkInit::GraphicsPipelineOutBundle vkInit::PipelineBuilder::build() {
  mPipelineInfo.pVertexInputState   = &mVertexInputInfo;
  mPipelineInfo.pInputAssemblyState = &mInputAssemblyInfo;

  make_viewport_state();
  mPipelineInfo.pViewportState = &mViewportState;

  mPipelineInfo.pRasterizationState = &mRasterizerInfo;

  mPipelineInfo.stageCount = mShaderStages.size();
  mPipelineInfo.pStages    = mShaderStages.data();

  mPipelineInfo.pMultisampleState = &mMultisamplingInfo;

  mPipelineInfo.pColorBlendState = &mColorBlendingInfo;

  vk::PipelineLayout pipelineLayout = make_pipeline_layout();
  mPipelineInfo.layout = pipelineLayout;

  vk::RenderPass renderpass = make_renderpass();
  mPipelineInfo.renderPass = renderpass;
  mPipelineInfo.subpass    = 0;

  vk::Pipeline graphicsPipeline;
  try {
    graphicsPipeline =
      mDevice.createGraphicsPipeline(nullptr, mPipelineInfo).value;
  } catch (vk::SystemError err) {
    printf("Error while creating pipeline. Error: %s\n", err.what());
  }

  GraphicsPipelineOutBundle output {};
  output.pipelineLayout   = pipelineLayout;
  output.renderPass       = renderpass;
  output.graphicsPipeline = graphicsPipeline;

  return output;
}

void vkInit::PipelineBuilder::add_descriptor_set_layout(
  vk::DescriptorSetLayout descriptorSetLayout
) {
  mDescriptorSetLayouts.push_back(descriptorSetLayout);
}

void vkInit::PipelineBuilder::reset_descriptor_set_layout() {
  mDescriptorSetLayouts.clear();
}

void vkInit::PipelineBuilder::reset_vertex_format() {
  mVertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
  mVertexInputInfo.vertexBindingDescriptionCount   = 0;
  mVertexInputInfo.pVertexBindingDescriptions      = nullptr;
  mVertexInputInfo.vertexAttributeDescriptionCount = 0;
  mVertexInputInfo.pVertexAttributeDescriptions    = nullptr;
}

void vkInit::PipelineBuilder::reset_shader_modules() {
  if (mVertexShader) {
    mDevice.destroyShaderModule(mVertexShader);
    mVertexShader = nullptr;
  }
  if (mFragmentShader) {
    mDevice.destroyShaderModule(mFragmentShader);
    mFragmentShader = nullptr;
  }
  mShaderStages.clear();
}

void vkInit::PipelineBuilder::reset_renderpass_attachments() {
  mAttachmentDescriptions.clear();
  mAttachmentReferences.clear();
}

vk::AttachmentDescription vkInit::PipelineBuilder::make_renderpass_attachment(
  const vk::Format& format,
  vk::AttachmentLoadOp loadOp,
  vk::AttachmentStoreOp storeOp,
  vk::ImageLayout initialLayout,
  vk::ImageLayout finalLayout
) {
  vk::AttachmentDescription attachment {};
  attachment.flags          = vk::AttachmentDescriptionFlags();
  attachment.format         = format;
  attachment.samples        = vk::SampleCountFlagBits::e1;
  attachment.loadOp         = loadOp;
  attachment.storeOp        = storeOp;
  attachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
  attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  attachment.initialLayout  = initialLayout;
  attachment.finalLayout    = finalLayout;

  return attachment;
}

vk::AttachmentReference vkInit::PipelineBuilder::make_attachment_reference(
  uint32_t attachmentIndex,
  vk::ImageLayout layout
) {
  vk::AttachmentReference attachmentRef {};
  attachmentRef.attachment = attachmentIndex;
  attachmentRef.layout     = layout;

  return attachmentRef;
}

void vkInit::PipelineBuilder::configure_input_assembly() {
  mInputAssemblyInfo.flags    = vk::PipelineInputAssemblyStateCreateFlags();
  mInputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
}

vk::PipelineShaderStageCreateInfo vkInit::PipelineBuilder::make_shader_info(
  const vk::ShaderModule& shaderModule,
  const vk::ShaderStageFlagBits& stage
) {
  vk::PipelineShaderStageCreateInfo shaderInfo {};
  shaderInfo.flags  = vk::PipelineShaderStageCreateFlags();
  shaderInfo.stage  = stage;
  shaderInfo.module = shaderModule,
  shaderInfo.pName  = "main";

  return shaderInfo;
}

vk::PipelineViewportStateCreateInfo 
vkInit::PipelineBuilder::make_viewport_state() {
  mViewport.x        = 0.0f;
  mViewport.y        = 0.0f;
  mViewport.width    = static_cast<float>(mSwapchainExtent.width);
  mViewport.height   = static_cast<float>(mSwapchainExtent.height);
  mViewport.minDepth = 0.0f;
  mViewport.maxDepth = 1.0f;

  mScissor.offset.x = 0.0f;
  mScissor.offset.y = 0.0f;
  mScissor.extent   = mSwapchainExtent;

  mViewportState.flags         = vk::PipelineViewportStateCreateFlags();
  mViewportState.viewportCount = 1;
  mViewportState.pViewports    = &mViewport;
  mViewportState.scissorCount  = 1;
  mViewportState.pScissors     = &mScissor;

  return mViewportState;
}

void vkInit::PipelineBuilder::make_rasterizer_info() {
  mRasterizerInfo.flags                   =
    vk::PipelineRasterizationStateCreateFlags();
  mRasterizerInfo.depthClampEnable        = VK_FALSE;
  mRasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
  mRasterizerInfo.polygonMode             = vk::PolygonMode::eFill;
  mRasterizerInfo.lineWidth               = 1.0f;
  mRasterizerInfo.cullMode                = vk::CullModeFlagBits::eBack;
  mRasterizerInfo.frontFace               = vk::FrontFace::eCounterClockwise;
  mRasterizerInfo.depthBiasEnable         = VK_FALSE;
}

void vkInit::PipelineBuilder::configure_multisampling() {
  mMultisamplingInfo.flags                =
    vk::PipelineMultisampleStateCreateFlags();
  mMultisamplingInfo.sampleShadingEnable  = VK_FALSE;
  mMultisamplingInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
}

void vkInit::PipelineBuilder::configure_color_blending() {
  mColorBlendAttachment.colorWriteMask =
    vk::ColorComponentFlagBits::eR |
    vk::ColorComponentFlagBits::eG |
    vk::ColorComponentFlagBits::eB |
    vk::ColorComponentFlagBits::eA;
  mColorBlendAttachment.blendEnable = VK_FALSE;

  mColorBlendingInfo.flags = vk::PipelineColorBlendStateCreateFlags();
  mColorBlendingInfo.logicOpEnable = VK_FALSE;
  mColorBlendingInfo.logicOp  =vk::LogicOp::eCopy;
  mColorBlendingInfo.attachmentCount = 1;
  mColorBlendingInfo.pAttachments = &mColorBlendAttachment;
  mColorBlendingInfo.blendConstants[0] = 0.0f;
  mColorBlendingInfo.blendConstants[1] = 0.0f;
  mColorBlendingInfo.blendConstants[2] = 0.0f;
  mColorBlendingInfo.blendConstants[3] = 0.0f;
}

vk::PipelineLayout vkInit::PipelineBuilder::make_pipeline_layout() {
  vk::PipelineLayoutCreateInfo layoutInfo {};
  layoutInfo.flags = vk::PipelineLayoutCreateFlags();
  layoutInfo.setLayoutCount =
    static_cast<uint32_t>(mDescriptorSetLayouts.size());
  layoutInfo.pSetLayouts = mDescriptorSetLayouts.data();
  layoutInfo.pushConstantRangeCount = 0;

  try {
    return mDevice.createPipelineLayout(layoutInfo);
  } catch (vk::SystemError err) {
    printf("Error while creating pipeline layout. Error: %s\n", err.what());
  }
  return nullptr;
}

vk::RenderPass vkInit::PipelineBuilder::make_renderpass() {
  mFlattenedAttachmentDescriptions.clear();
  mFlattenedAttachmentReferences.clear();
  size_t attachmentCount = mAttachmentDescriptions.size();
  mFlattenedAttachmentDescriptions.resize(attachmentCount);
  mFlattenedAttachmentReferences.resize(attachmentCount);

  for (size_t i = 0; i < attachmentCount; ++i) {
    mFlattenedAttachmentDescriptions[i] = mAttachmentDescriptions[i];
    mFlattenedAttachmentReferences[i] = mAttachmentReferences[i];
  }

  vk::SubpassDescription subpass = make_subpass(mFlattenedAttachmentReferences);
  vk::RenderPassCreateInfo renderpassInfo =
    make_renderpass_info(mFlattenedAttachmentDescriptions, subpass);

  try {
    return mDevice.createRenderPass(renderpassInfo);
  } catch (vk::SystemError err) {
    printf("Error while creating render pass. Error: %s\n", err.what());
  }
  return nullptr;
}

vk::SubpassDescription vkInit::PipelineBuilder::make_subpass(
  const std::vector<vk::AttachmentReference>& attachments
) {
  vk::SubpassDescription subpass {};
  subpass.flags                = vk::SubpassDescriptionFlags();
  subpass.pipelineBindPoint    = vk::PipelineBindPoint::eGraphics;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments    = &attachments[0];

  if (attachments.size() > 1) {
    subpass.pDepthStencilAttachment = &attachments[1];
  } else {
    subpass.pDepthStencilAttachment = nullptr;
  }

  return subpass;
}

vk::RenderPassCreateInfo vkInit::PipelineBuilder::make_renderpass_info(
  const std::vector<vk::AttachmentDescription>& attachments,
  const vk::SubpassDescription& subpass
) {
  vk::RenderPassCreateInfo renderpassInfo {};
  renderpassInfo.flags = vk::RenderPassCreateFlags();
  renderpassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderpassInfo.pAttachments = attachments.data();
  renderpassInfo.subpassCount = 1;
  renderpassInfo.pSubpasses = &subpass;

  return renderpassInfo;
}
