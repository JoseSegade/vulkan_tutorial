// Copyright (c) 2024 Meerkat
#ifndef INC_FRAME_H_
#define INC_FRAME_H_

#include "Common.h"
#include "Memory.h"
#include "Pipeline.h"
#include <unordered_map>
#include <vector>

namespace vkUtil {

struct CameraMatrices {
  glm::mat4 view;
  glm::mat4 projection;
  glm::mat4 viewProjection;
};

struct CameraVectors {
  glm::vec4 forwards;
  glm::vec4 right;
  glm::vec4 up;
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
  void record_write_operations();
  void destroy();

 public:
  // Devices
  vk::Device               mDevice;
  vk::PhysicalDevice       mPhysicalDevice;

  // Swapchain
  vk::Image                mImage;
  vk::ImageView            mImageView;

  std::unordered_map<PipelineTypes, vk::Framebuffer> mFramebuffer;

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

  CameraMatrices           mCameraMatrixData;
  Buffer                   mCameraMatrixBuffer;
  void*                    mCameraMatrixWriteLocation;

  CameraVectors            mCameraVectorsData;
  Buffer                   mCameraVectorsBuffer;
  void*                    mCameraVectorsWriteLocation;

  std::vector<glm::mat4>   mModelTransforms;
  Buffer                   mModelBuffer;
  void*                    mModelBufferWriteLocation;

  vk::DescriptorBufferInfo mCameraMatrixDescriptor;
  vk::DescriptorBufferInfo mCameraVectorsDescriptor;
  vk::DescriptorBufferInfo mModelBufferDescriptor;

  std::unordered_map<PipelineTypes, vk::DescriptorSet> mDescriptorSet;
  std::vector<vk::WriteDescriptorSet>                  mWriteOps;
};

}  // namespace vkUtil

#endif  // INC_FRAME_H_
