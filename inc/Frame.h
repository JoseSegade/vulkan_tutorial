// Copyright (c) 2024 Meerkat
#ifndef INC_FRAME_H_
#define INC_FRAME_H_

#include "Common.h"

namespace vkUtil {

struct SwapChainFrame {
  vk::Image         image;
  vk::ImageView     imageView;
  vk::Framebuffer   framebuffer;
  vk::CommandBuffer commandBuffer;
  vk::Semaphore     imageAvailable;
  vk::Semaphore     renderFinished;
  vk::Fence         inFlight;
};

}  // namespace vkUtil

#endif  // INC_FRAME_H_
