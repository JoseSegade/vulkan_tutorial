// Copyright (c) 2024 Meerkat
#ifndef INC_FRAMEBUFFER_H_
#define INC_FRAMEBUFFER_H_

#include "Common.h"
#include "Frame.h"
#include "Pipeline.h"
#include <unordered_map>
#include <vector>
#include <string>

namespace vkInit {

struct FramebufferInput {
  vk::Device                                        device;
  std::unordered_map<PipelineTypes, vk::RenderPass> renderPass;
  vk::Extent2D                                      swapchainExtent;
};

inline void make_framebuffers(
  const FramebufferInput& input,
  std::vector<vkUtil::SwapChainFrame>* frames,
  bool debug) {
  for (size_t i = 0; i < frames->size(); ++i) {
    {
      std::vector<vk::ImageView> attachments {
        frames->at(i).mImageView
      };

      vk::FramebufferCreateInfo framebufferInfo {};
      framebufferInfo.flags           = vk::FramebufferCreateFlags();
      framebufferInfo.renderPass      =
        input.renderPass.at(PipelineTypes::SKY);
      framebufferInfo.attachmentCount = attachments.size();
      framebufferInfo.pAttachments    = attachments.data();
      framebufferInfo.width           = input.swapchainExtent.width;
      framebufferInfo.height          = input.swapchainExtent.height;
      framebufferInfo.layers          = 1;

      try {
        if (debug) {
          printf("Creating sky framebuffer %ld\n", i);
        }
        frames->at(i).mFramebuffer[PipelineTypes::SKY] =
          input.device.createFramebuffer(framebufferInfo);
      } catch (vk::SystemError err) {
        if (debug) {
          printf("Error creating sky framebuffer %ld. Error: %s\n",
                 i, err.what());
        }
      }
    }
    {
      std::vector<vk::ImageView> attachments {
        frames->at(i).mImageView,
        frames->at(i).mDepthBufferView
      };

      vk::FramebufferCreateInfo framebufferInfo {};
      framebufferInfo.flags           = vk::FramebufferCreateFlags();
      framebufferInfo.renderPass      =
        input.renderPass.at(PipelineTypes::STANDARD);
      framebufferInfo.attachmentCount = attachments.size();
      framebufferInfo.pAttachments    = attachments.data();
      framebufferInfo.width           = input.swapchainExtent.width;
      framebufferInfo.height          = input.swapchainExtent.height;
      framebufferInfo.layers          = 1;

      try {
        if (debug) {
          printf("Creating framebuffer %ld\n", i);
        }
        frames->at(i).mFramebuffer[PipelineTypes::STANDARD] =
          input.device.createFramebuffer(framebufferInfo);
      } catch (vk::SystemError err) {
        if (debug) {
          printf("Error creating framebuffer %ld. Error: %s\n",
                 i, err.what());
        }
      }
    }
  }
}

}  // namespace vkInit

#endif  // INC_FRAMEBUFFER_H_
