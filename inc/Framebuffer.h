// Copyright (c) 2024 Meerkat
#ifndef INC_FRAMEBUFFER_H_
#define INC_FRAMEBUFFER_H_

#include "Common.h"
#include "Frame.h"
#include <vector>
#include <string>

namespace vkInit {

struct FramebufferInput {
  vk::Device     device;
  vk::RenderPass renderPass;
  vk::Extent2D   swapchainExtent;
};

inline void make_framebuffers(
  const FramebufferInput& input,
  std::vector<vkUtil::SwapChainFrame>* frames,
  bool debug) {
  for (size_t i = 0; i < frames->size(); ++i) {
    std::vector<vk::ImageView> attachments {
      frames->at(i).imageView
    };

    vk::FramebufferCreateInfo framebufferInfo {};
    framebufferInfo.flags           = vk::FramebufferCreateFlags();
    framebufferInfo.renderPass      = input.renderPass;
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments    = attachments.data();
    framebufferInfo.width           = input.swapchainExtent.width;
    framebufferInfo.height          = input.swapchainExtent.height;
    framebufferInfo.layers          = 1;

    try {
      if (debug) {
        printf("Creating framebuffer %ld\n", i);
      }
      frames->at(i).framebuffer =
        input.device.createFramebuffer(framebufferInfo);
    } catch (vk::SystemError err) {
      if (debug) {
        printf("Error creating framebuffer %ld. Error: %s\n",
               i, err.what());
      }
    }
  }
}

}  // namespace vkInit


#endif  // INC_FRAMEBUFFER_H_
