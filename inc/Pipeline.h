// Copyright (c) 2024 Meerkat
#ifndef INC_PIPELINE_H_
#define INC_PIPELINE_H_

#include "Common.h"
#include "Shaders.h"
#include <vector>
#include <string>

namespace vkInit {

struct GraphicsPipelineInBundle {
  vk::Device   device;
  std::string  vertexFilepath;
  std::string  fragmenFilepath;
  vk::Extent2D extent;
  vk::Format   swapchainFormat;
};

struct GraphicsPipelineOutBundle {
  vk::PipelineLayout pipelineLayout;
  vk::RenderPass     renderPass;
  vk::Pipeline       graphicsPipeline;
};

inline vk::PipelineLayout make_pipeline_layout(
  vk::Device device, bool debug) {
  vk::PipelineLayoutCreateInfo layoutInfo {};
  layoutInfo.flags = vk::PipelineLayoutCreateFlags();
  layoutInfo.setLayoutCount = 0;
  layoutInfo.pushConstantRangeCount = 0;

  vk::PipelineLayout pl {};
  try {
    pl = device.createPipelineLayout(layoutInfo);
  } catch (vk::SystemError err) {
    if (debug) {
      printf("Failed to create pipeline layout. Error: %s\n",
             err.what());
    }
  }
  return pl;
}

inline vk::RenderPass make_renderpass(
  vk::Device device, vk::Format swapchainImageFormat,
  bool debug) {
  vk::AttachmentDescription colorAttachment {};
  colorAttachment.flags          = vk::AttachmentDescriptionFlags();
  colorAttachment.format         = swapchainImageFormat;
  colorAttachment.samples        = vk::SampleCountFlagBits::e1;
  colorAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
  colorAttachment.storeOp        = vk::AttachmentStoreOp::eStore;
  colorAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
  colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  colorAttachment.initialLayout  = vk::ImageLayout::eUndefined;
  colorAttachment.finalLayout    = vk::ImageLayout::ePresentSrcKHR;

  vk::AttachmentReference colorAttachmentRef {};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout     = vk::ImageLayout::eColorAttachmentOptimal;

  vk::SubpassDescription subpass {};
  subpass.flags                = vk::SubpassDescriptionFlags();
  subpass.pipelineBindPoint    = vk::PipelineBindPoint::eGraphics;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments    = &colorAttachmentRef;

  vk::RenderPassCreateInfo renderPassInfo {};
  renderPassInfo.flags = vk::RenderPassCreateFlags();
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;

  vk::RenderPass rp {};
  try {
    rp = device.createRenderPass(renderPassInfo);
  } catch (vk::SystemError err) {
    if (debug) {
      printf("Filed to create render pass. Error %s\n", err.what());
    }
  }

  return rp;
}

inline GraphicsPipelineOutBundle make_graphics_pipeline(
  const GraphicsPipelineInBundle& specification,
  bool debug) {
  GraphicsPipelineOutBundle out {};

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo {};
  vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
  vertexInputInfo.vertexBindingDescriptionCount   = 0;
  vertexInputInfo.vertexAttributeDescriptionCount = 0;

  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo {};
  inputAssemblyInfo.flags    = vk::PipelineInputAssemblyStateCreateFlags();
  inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;

  std::vector<vk::PipelineShaderStageCreateInfo> shaderStages {};

  if (debug) {
    printf("Creating vertex shader module...\n");
  }
  vk::ShaderModule vertexShader =
    vkUtil::createModule(
      specification.vertexFilepath,
      specification.device,
      debug);

  vk::PipelineShaderStageCreateInfo vertexShaderInfo {};
  vertexShaderInfo.flags  = vk::PipelineShaderStageCreateFlags();
  vertexShaderInfo.stage  = vk::ShaderStageFlagBits::eVertex;
  vertexShaderInfo.module = vertexShader;
  vertexShaderInfo.pName  = "main";
  shaderStages.push_back(vertexShaderInfo);

  if (debug) {
    printf("Creating fragment shader module...\n");
  }
  vk::ShaderModule fragmentShader =
    vkUtil::createModule(
      specification.fragmenFilepath,
      specification.device,
      debug);

  vk::PipelineShaderStageCreateInfo fragmentShaderInfo {};
  fragmentShaderInfo.flags  = vk::PipelineShaderStageCreateFlags();
  fragmentShaderInfo.stage  = vk::ShaderStageFlagBits::eFragment;
  fragmentShaderInfo.module = fragmentShader;
  fragmentShaderInfo.pName  = "main";
  shaderStages.push_back(fragmentShaderInfo);

  vk::Viewport viewport {};
  viewport.x        = 0.0f;
  viewport.y        = 0.0f;
  viewport.width    = specification.extent.width;
  viewport.height   = specification.extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  vk::Rect2D scissor {};
  scissor.offset.x = 0.0f;
  scissor.offset.y = 0.0f;
  scissor.extent   = specification.extent;

  vk::PipelineViewportStateCreateInfo viewportState {};
  viewportState.flags =
    vk::PipelineViewportStateCreateFlags();
  viewportState.viewportCount = 1;
  viewportState.pViewports    = &viewport;
  viewportState.scissorCount  = 1;
  viewportState.pScissors     = &scissor;

  vk::PipelineRasterizationStateCreateInfo rasterizer {};
  rasterizer.flags =
    vk::PipelineRasterizationStateCreateFlags();
  rasterizer.depthClampEnable        = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode             = vk::PolygonMode::eFill;
  rasterizer.lineWidth               = 1.0f;
  rasterizer.cullMode                = vk::CullModeFlagBits::eBack;
  rasterizer.frontFace               = vk::FrontFace::eClockwise;
  rasterizer.depthBiasEnable         = VK_FALSE;

  vk::PipelineMultisampleStateCreateInfo multisampling {};
  multisampling.flags = vk::PipelineMultisampleStateCreateFlags();
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

  vk::PipelineColorBlendAttachmentState colorBlendAttachment {};
  colorBlendAttachment.colorWriteMask =
    vk::ColorComponentFlagBits::eR |
    vk::ColorComponentFlagBits::eG |
    vk::ColorComponentFlagBits::eB |
    vk::ColorComponentFlagBits::eA;
  colorBlendAttachment.blendEnable = VK_FALSE;

  vk::PipelineColorBlendStateCreateInfo colorBlending {};
  colorBlending.flags             = vk::PipelineColorBlendStateCreateFlags();
  colorBlending.logicOpEnable     = VK_FALSE;
  colorBlending.logicOp           = vk::LogicOp::eCopy;
  colorBlending.attachmentCount   = 1;
  colorBlending.pAttachments      = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  if (debug) {
    printf("Creating pipeline layout...\n");
  }
  vk::PipelineLayout pipelineLayout =
    make_pipeline_layout(specification.device, debug);

  if (debug) {
    printf("Creating render pass...\n");
  }
  vk::RenderPass renderPass =
    make_renderpass(specification.device, specification.swapchainFormat, debug);


  vk::GraphicsPipelineCreateInfo pipelineInfo {};
  pipelineInfo.flags               = vk::PipelineCreateFlags();
  pipelineInfo.pVertexInputState   = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
  pipelineInfo.pViewportState      = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.stageCount          =
    static_cast<uint32_t>(shaderStages.size());
  pipelineInfo.pStages             = shaderStages.data();
  pipelineInfo.pMultisampleState   = &multisampling;
  pipelineInfo.pColorBlendState    = &colorBlending;
  pipelineInfo.layout              = pipelineLayout;
  pipelineInfo.renderPass          = renderPass;
  pipelineInfo.subpass             = 0;
  pipelineInfo.basePipelineHandle  = nullptr;

  if (debug) {
    printf("Creating graphics pipeline...\n");
  }
  vk::Pipeline graphicsPipeline {};
  try {
    graphicsPipeline =
      (specification.device.createGraphicsPipeline(nullptr, pipelineInfo))
        .value;
  } catch (vk::SystemError err) {
    if (debug) {
      printf("Failed to create Graphics Pipeline. Error: %s",
             err.what());
    }
  }

  out.pipelineLayout   = pipelineLayout;
  out.renderPass       = renderPass;
  out.graphicsPipeline = graphicsPipeline;

  specification.device.destroyShaderModule(vertexShader);
  specification.device.destroyShaderModule(fragmentShader);

  return out;
}

}  // namespace vkInit

#endif  // INC_PIPELINE_H_
