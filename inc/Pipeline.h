// Copyright (c) 2024 Meerkat
#ifndef INC_PIPELINE_H_
#define INC_PIPELINE_H_

#include "Common.h"
#include "Shaders.h"
#include "RenderStructs.h"
#include "Mesh.h"
#include <unordered_map>
#include <vector>
#include <string>

enum class PipelineTypes {
  SKY,
  STANDARD,
};

const static std::vector<PipelineTypes> sPipelineTypes = { {
  PipelineTypes::SKY,
  PipelineTypes::STANDARD
} };

namespace vkInit {

struct GraphicsPipelineOutBundle {
  vk::PipelineLayout pipelineLayout;
  vk::RenderPass     renderPass;
  vk::Pipeline       graphicsPipeline;
};

class PipelineBuilder {
 public:
  PipelineBuilder();
  ~PipelineBuilder();

  void init(vk::Device);

  void reset();

  void specify_vertex_format(
    vk::VertexInputBindingDescription bindingDescription,
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions
  );

  void specify_vertex_shader(const char* filename);
  void specify_fragment_shader(const char* filename);
  void specify_swapchain_extent(vk::Extent2D screen_size);
  void specify_depth_attachment(
    const vk::Format& depthFormat,
    uint32_t attachment_index
  );
  void clear_depth_attachment();
  void add_color_attachment(
    const vk::Format& format,
    uint32_t attachment_index
  );
  void set_overwrite_mode(bool mode);
  GraphicsPipelineOutBundle build();
  void add_descriptor_set_layout(vk::DescriptorSetLayout descriptorSetLayout);
  void reset_descriptor_set_layout();

 private:
  vk::Device                     mDevice;
  vk::GraphicsPipelineCreateInfo mPipelineInfo {};

  vk::VertexInputBindingDescription                mBindingDescription;
  std::vector<vk::VertexInputAttributeDescription> mAttributeDescriptions;
  vk::PipelineVertexInputStateCreateInfo           mVertexInputInfo {};
  vk::PipelineInputAssemblyStateCreateInfo         mInputAssemblyInfo {};

  std::vector<vk::PipelineShaderStageCreateInfo>   mShaderStages;
  vk::ShaderModule                                 mVertexShader = nullptr;
  vk::ShaderModule                                 mFragmentShader = nullptr;
  vk::PipelineShaderStageCreateInfo                mVertexShaderInfo;
  vk::PipelineShaderStageCreateInfo                mFragmentShaderInfo;

  vk::Extent2D                                     mSwapchainExtent;
  vk::Viewport                                     mViewport {};
  vk::Rect2D                                       mScissor {};
  vk::PipelineViewportStateCreateInfo              mViewportState {};

  vk::PipelineRasterizationStateCreateInfo         mRasterizerInfo {};

  vk::PipelineDepthStencilStateCreateInfo          mDepthState;
  std::unordered_map<uint32_t, vk::AttachmentDescription>
    mAttachmentDescriptions;
  std::unordered_map<uint32_t, vk::AttachmentReference>
    mAttachmentReferences;
  std::vector<vk::AttachmentDescription>  mFlattenedAttachmentDescriptions;
  std::vector<vk::AttachmentReference>    mFlattenedAttachmentReferences;
  vk::PipelineMultisampleStateCreateInfo  mMultisamplingInfo {};
  vk::PipelineColorBlendAttachmentState   mColorBlendAttachment {};
  vk::PipelineColorBlendStateCreateInfo   mColorBlendingInfo {};
  std::vector<vk::DescriptorSetLayout>    mDescriptorSetLayouts;
  bool                                    mOverwrite;

 private:
  void reset_vertex_format();
  void reset_shader_modules();
  void reset_renderpass_attachments();
  vk::AttachmentDescription make_renderpass_attachment(
    const vk::Format& format,
    vk::AttachmentLoadOp loadOp,
    vk::AttachmentStoreOp storeOp,
    vk::ImageLayout initialLayout,
    vk::ImageLayout finalLayout
  );
  vk::AttachmentReference make_attachment_reference(
    uint32_t attachmentIndex,
    vk::ImageLayout layout
  );
  void configure_input_assembly();
  vk::PipelineShaderStageCreateInfo make_shader_info(
    const vk::ShaderModule& shaderModule,
    const vk::ShaderStageFlagBits& stage
  );
  vk::PipelineViewportStateCreateInfo make_viewport_state();
  void make_rasterizer_info();
  void configure_multisampling();
  void configure_color_blending();
  vk::PipelineLayout make_pipeline_layout();
  vk::RenderPass make_renderpass();
  vk::SubpassDescription make_subpass(
    const std::vector<vk::AttachmentReference>& attachments
  );
  vk::RenderPassCreateInfo make_renderpass_info(
    const std::vector<vk::AttachmentDescription>& attachments,
    const vk::SubpassDescription& subpass
  );
};

}

#endif  // INC_PIPELINE_H_
