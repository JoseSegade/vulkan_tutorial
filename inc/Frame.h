// Copyright (c) 2024 Meerkat
#ifndef INC_FRAME_H_
#define INC_FRAME_H_

#include "Common.h"
#include "Memory.h"
#include <vector>

namespace vkUtil {

struct UniformBufferObject {
  glm::mat4 view;
  glm::mat4 projection;
  glm::mat4 viewProjection;
};

class SwapChainFrame {
 public:
  struct SyncObjects {
    vk::Semaphore imageAvailable;
    vk::Semaphore renderFinished;
    vk::Fence     inFlight;
  };

 public:
  SwapChainFrame();
  ~SwapChainFrame();

  void make_descriptor_resources();
  void make_depth_resources();
  void write_descriptor_set();
  void destroy();

 public:
  // Devices
  vk::Device               mDevice;
  vk::PhysicalDevice       mPhysicalDevice;

  // Swapchain
  vk::Image                mImage;
  vk::ImageView            mImageView;
  vk::Framebuffer          mFramebuffer;
  vk::Image                mDepthBuffer;
  vk::DeviceMemory         mDepthBufferMemory;
  vk::ImageView            mDepthBufferView;
  vk::Format               mDepthFormat;
  uint32_t                 mWidth;
  uint32_t                 mHeight;

  // Command
  vk::CommandBuffer        mCommandBuffer;

  // Sync
  vk::Semaphore            mImageAvailable;
  vk::Semaphore            mRenderFinished;
  vk::Fence                mInFlight;

  // Resources
  UniformBufferObject      mCameraData;
  Buffer                   mCameraDataBuffer;
  void*                    mCameraDataWriteLocation;
  std::vector<glm::mat4>   mModelTransforms;
  Buffer                   mModelBuffer;
  void*                    mModelBufferWriteLocation;

  // Resource descriptors
  vk::DescriptorBufferInfo mUniformBufferDescriptor;
  vk::DescriptorBufferInfo mModelBufferDescriptor;
  vk::DescriptorSet        mDescriptorSet;
};

}  // namespace vkUtil

#endif  // INC_FRAME_H_
