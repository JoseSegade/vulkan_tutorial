// Copyright (c) 2024 Meerkat
#include "../inc/Frame.h"
#include "../inc/Image.h"

vkUtil::SwapChainFrame::SwapChainFrame() {
}

vkUtil::SwapChainFrame::~SwapChainFrame() {
}

void vkUtil::SwapChainFrame::make_descriptor_resources() {
  BufferInputChunk input {};

  {
    input.physicalDevice   = mPhysicalDevice;
    input.device           = mDevice;
    input.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible
      | vk::MemoryPropertyFlagBits::eHostCoherent;
    input.size             = sizeof(CameraMatrices);
    input.usage            = vk::BufferUsageFlagBits::eUniformBuffer;

    mCameraMatrixBuffer = createBuffer(input);

    mCameraMatrixWriteLocation = mDevice.mapMemory(
      mCameraMatrixBuffer.bufferMemory,
      0,
      sizeof(CameraMatrices)
    );
  }

  {
    input.size             = sizeof(CameraVectors);

    mCameraVectorsBuffer = createBuffer(input);

    mCameraVectorsWriteLocation = mDevice.mapMemory(
      mCameraVectorsBuffer.bufferMemory,
      0,
      sizeof(CameraVectors)
    );
  }

  input.size             = 1024 * sizeof(glm::mat4);
  input.usage            = vk::BufferUsageFlagBits::eStorageBuffer;

  mModelBuffer = createBuffer(input);

  mModelBufferWriteLocation = mDevice.mapMemory(
    mModelBuffer.bufferMemory, 0, 1024 * sizeof(glm::mat4));

  mModelTransforms.reserve(1024);
  for (uint32_t i = 0; i < 1024; ++i) {
    mModelTransforms.push_back(glm::mat4(1.0f));
  }

  mCameraMatrixDescriptor.buffer = mCameraMatrixBuffer.buffer;
  mCameraMatrixDescriptor.offset = 0;
  mCameraMatrixDescriptor.range  = sizeof(CameraMatrices);

  mCameraVectorsDescriptor.buffer = mCameraVectorsBuffer.buffer;
  mCameraVectorsDescriptor.offset = 0;
  mCameraVectorsDescriptor.range  = sizeof(CameraVectors);

  mModelBufferDescriptor.buffer = mModelBuffer.buffer;
  mModelBufferDescriptor.offset = 0;
  mModelBufferDescriptor.range  = 1024 * sizeof(glm::mat4);
}

void vkUtil::SwapChainFrame::make_depth_resources() {
  mDepthFormat = vkImage::find_supported_format(
    mPhysicalDevice,
    { vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint },
    vk::ImageTiling::eOptimal,
    vk::FormatFeatureFlagBits::eDepthStencilAttachment);

  vkImage::ImageInputChunk imageInfo {};
  imageInfo.device           = mDevice;
  imageInfo.physicalDevice   = mPhysicalDevice;
  imageInfo.tiling           = vk::ImageTiling::eOptimal;
  imageInfo.usage            = vk::ImageUsageFlagBits::eDepthStencilAttachment;
  imageInfo.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
  imageInfo.width            = mWidth;
  imageInfo.height           = mHeight;
  imageInfo.arraySize        = 1;
  imageInfo.format           = mDepthFormat;

  mDepthBuffer       = vkImage::make_image(imageInfo);
  mDepthBufferMemory = vkImage::make_image_memory(imageInfo, mDepthBuffer);
  mDepthBufferView   = vkImage::make_image_view(
    mDevice,
    mDepthBuffer,
    mDepthFormat,
    vk::ImageAspectFlagBits::eDepth,
    vk::ImageViewType::e2D,
    1
  );
}

void vkUtil::SwapChainFrame::write_descriptor_set() {
  mDevice.updateDescriptorSets(mWriteOps, nullptr);
}

void vkUtil::SwapChainFrame::record_write_operations() {
  vk::WriteDescriptorSet cameraVectorWrite {};
  cameraVectorWrite.dstSet          = mDescriptorSet[PipelineTypes::SKY];
  cameraVectorWrite.dstBinding      = 0;
  cameraVectorWrite.dstArrayElement = 0;
  cameraVectorWrite.descriptorCount = 1;
  cameraVectorWrite.descriptorType  = vk::DescriptorType::eUniformBuffer;
  cameraVectorWrite.pBufferInfo     = &mCameraVectorsDescriptor;

  vk::WriteDescriptorSet cameraMatrixWrite {};
  cameraMatrixWrite.dstSet          = mDescriptorSet[PipelineTypes::STANDARD];
  cameraMatrixWrite.dstBinding      = 0;
  cameraMatrixWrite.dstArrayElement = 0;
  cameraMatrixWrite.descriptorCount = 1;
  cameraMatrixWrite.descriptorType  = vk::DescriptorType::eUniformBuffer;
  cameraMatrixWrite.pBufferInfo     = &mCameraMatrixDescriptor;

  vk::WriteDescriptorSet ssboWrite {};
  ssboWrite.dstSet          = mDescriptorSet[PipelineTypes::STANDARD];
  ssboWrite.dstBinding      = 1;
  ssboWrite.dstArrayElement = 0;
  ssboWrite.descriptorCount = 1;
  ssboWrite.descriptorType  = vk::DescriptorType::eStorageBuffer;
  ssboWrite.pBufferInfo     = &mModelBufferDescriptor;

  mWriteOps = { {
    cameraVectorWrite,
    cameraMatrixWrite,
    ssboWrite
  } };
}

void vkUtil::SwapChainFrame::destroy() {
  mDevice.destroyImage(mDepthBuffer);
  mDevice.freeMemory(mDepthBufferMemory);
  mDevice.destroyImageView(mDepthBufferView);

  mDevice.destroyImageView(mImageView);
  for (PipelineTypes pt : sPipelineTypes) {
    mDevice.destroyFramebuffer(mFramebuffer[pt]);
  }
  mDevice.destroyFence(mInFlight);
  mDevice.destroySemaphore(mImageAvailable);
  mDevice.destroySemaphore(mRenderFinished);

  mDevice.unmapMemory(mCameraMatrixBuffer.bufferMemory);
  mDevice.freeMemory(mCameraMatrixBuffer.bufferMemory);
  mDevice.destroyBuffer(mCameraMatrixBuffer.buffer);

  mDevice.unmapMemory(mCameraVectorsBuffer.bufferMemory);
  mDevice.freeMemory(mCameraVectorsBuffer.bufferMemory);
  mDevice.destroyBuffer(mCameraVectorsBuffer.buffer);

  mDevice.unmapMemory(mModelBuffer.bufferMemory);
  mDevice.freeMemory(mModelBuffer.bufferMemory);
  mDevice.destroyBuffer(mModelBuffer.buffer);
}
