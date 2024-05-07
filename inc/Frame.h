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

  void init(vk::Device device, vk::PhysicalDevice physicalDevice);

  void make_descriptor_resources();
  void make_depth_resources();
  void write_descriptor_set();
  void destroy();

  void setSyncObjects(const SyncObjects& syncObjects);
  void setImage(const vk::Image& image);
  void setImageView(const vk::ImageView& imageView);
  void setDescriptorSet(const vk::DescriptorSet& descriptorSet);
  void setDimensions(uint32_t width, uint32_t height);
  void setCommandBuffer(const vk::CommandBuffer& commandBuffer);
  void setFramebuffer(const vk::Framebuffer& framebuffer);

  UniformBufferObject* const CameraData();
  void* const CameraDataWriteLocation();
  std::vector<glm::mat4>& ModelTransforms();
  void* const ModelBufferWriteLocation();
  vk::ImageView getImageView();
  vk::ImageView getDepthBufferView();
  vk::Fence getInFlight();
  vk::Semaphore getImageAvailable();
  vk::Semaphore getRenderFinished();
  vk::CommandBuffer getCommandBuffer();
  vk::Framebuffer getFramebuffer();
  vk::DescriptorSet getDescriptorSet();
  vk::Format getDepthFormat();

 private:
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
