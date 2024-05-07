// Copyright (c) 2024 Meerkat
#include "../inc/Frame.h"
#include "../inc/Image.h"

vkUtil::SwapChainFrame::SwapChainFrame() {
}

vkUtil::SwapChainFrame::~SwapChainFrame() {
}

void vkUtil::SwapChainFrame::init(
  vk::Device device, vk::PhysicalDevice physicalDevice) {
  mDevice = device;
  mPhysicalDevice = physicalDevice;
}

void vkUtil::SwapChainFrame::setSyncObjects(
  const SyncObjects& syncObjects) {
  mImageAvailable = syncObjects.imageAvailable;
  mRenderFinished = syncObjects.renderFinished;
  mInFlight       = syncObjects.inFlight;
}

void vkUtil::SwapChainFrame::setImage(const vk::Image& value) {
  mImage = value;
}

void vkUtil::SwapChainFrame::setImageView(const vk::ImageView& value) {
  mImageView = value;
}

void vkUtil::SwapChainFrame::setDescriptorSet(const vk::DescriptorSet& value) {
  mDescriptorSet = value;
}

void vkUtil::SwapChainFrame::setDimensions(uint32_t width, uint32_t height) {
  mWidth = width;
  mHeight = height;
}

vkUtil::UniformBufferObject* const vkUtil::SwapChainFrame::CameraData() {
  return &mCameraData;
}

void vkUtil::SwapChainFrame::setCommandBuffer(const vk::CommandBuffer& value) {
  mCommandBuffer = value;
}

void vkUtil::SwapChainFrame::setFramebuffer(const vk::Framebuffer& value) {
  mFramebuffer = value;
}

void* const vkUtil::SwapChainFrame::CameraDataWriteLocation() {
  return mCameraDataWriteLocation;
}

std::vector<glm::mat4>& vkUtil::SwapChainFrame::ModelTransforms() {
  return mModelTransforms;
}

void* const vkUtil::SwapChainFrame::ModelBufferWriteLocation() {
  return mModelBufferWriteLocation;
}

vk::ImageView vkUtil::SwapChainFrame::getImageView() {
  return mImageView;
}

vk::ImageView vkUtil::SwapChainFrame::getDepthBufferView() {
  return mDepthBufferView;
}

vk::Fence vkUtil::SwapChainFrame::getInFlight() {
  return mInFlight;
}

vk::Semaphore vkUtil::SwapChainFrame::getImageAvailable() {
  return mImageAvailable;
}

vk::Semaphore vkUtil::SwapChainFrame::getRenderFinished() {
  return mRenderFinished;
}

vk::CommandBuffer vkUtil::SwapChainFrame::getCommandBuffer() {
  return mCommandBuffer;
}

vk::Framebuffer vkUtil::SwapChainFrame::getFramebuffer() {
  return mFramebuffer;
}

vk::DescriptorSet vkUtil::SwapChainFrame::getDescriptorSet() {
  return mDescriptorSet;
}

vk::Format vkUtil::SwapChainFrame::getDepthFormat() {
  return mDepthFormat;
}

void vkUtil::SwapChainFrame::make_descriptor_resources() {
  BufferInputChunk input {};
  input.physicalDevice   = mPhysicalDevice;
  input.device           = mDevice;
  input.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible
    | vk::MemoryPropertyFlagBits::eHostCoherent;
  input.size             = sizeof(UniformBufferObject);
  input.usage            = vk::BufferUsageFlagBits::eUniformBuffer;

  mCameraDataBuffer = createBuffer(input);

  mCameraDataWriteLocation = mDevice.mapMemory(
    mCameraDataBuffer.bufferMemory, 0, sizeof(UniformBufferObject));

  input.size             = 1024 * sizeof(glm::mat4);
  input.usage            = vk::BufferUsageFlagBits::eStorageBuffer;

  mModelBuffer = createBuffer(input);

  mModelBufferWriteLocation = mDevice.mapMemory(
    mModelBuffer.bufferMemory, 0, 1024 * sizeof(glm::mat4));

  mModelTransforms.reserve(1024);
  for (uint32_t i = 0; i < 1024; ++i) {
    mModelTransforms.push_back(glm::mat4(1.0f));
  }

  mUniformBufferDescriptor.buffer = mCameraDataBuffer.buffer;
  mUniformBufferDescriptor.offset = 0;
  mUniformBufferDescriptor.range  = sizeof(UniformBufferObject);

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
  imageInfo.format           = mDepthFormat;

  mDepthBuffer       = vkImage::make_image(imageInfo);
  mDepthBufferMemory = vkImage::make_image_memory(imageInfo, mDepthBuffer);
  mDepthBufferView   = vkImage::make_image_view(
    mDevice, mDepthBuffer, mDepthFormat, vk::ImageAspectFlagBits::eDepth);
}

void vkUtil::SwapChainFrame::write_descriptor_set() {
  vk::WriteDescriptorSet writeInfo {};
  writeInfo.dstSet          = mDescriptorSet;
  writeInfo.dstBinding      = 0;
  writeInfo.dstArrayElement = 0;
  writeInfo.descriptorCount = 1;
  writeInfo.descriptorType  = vk::DescriptorType::eUniformBuffer;
  writeInfo.pBufferInfo     = &mUniformBufferDescriptor;

  mDevice.updateDescriptorSets(writeInfo, nullptr);

  vk::WriteDescriptorSet writeInfo2 {};
  writeInfo2.dstSet          = mDescriptorSet;
  writeInfo2.dstBinding      = 1;
  writeInfo2.dstArrayElement = 0;
  writeInfo2.descriptorCount = 1;
  writeInfo2.descriptorType  = vk::DescriptorType::eStorageBuffer;
  writeInfo2.pBufferInfo     = &mModelBufferDescriptor;

  mDevice.updateDescriptorSets(writeInfo2, nullptr);
}

void vkUtil::SwapChainFrame::destroy() {
  mDevice.destroyImage(mDepthBuffer);
  mDevice.freeMemory(mDepthBufferMemory);
  mDevice.destroyImageView(mDepthBufferView);

  mDevice.destroyImageView(mImageView);
  mDevice.destroyFramebuffer(mFramebuffer);
  mDevice.destroyFence(mInFlight);
  mDevice.destroySemaphore(mImageAvailable);
  mDevice.destroySemaphore(mRenderFinished);

  mDevice.unmapMemory(mCameraDataBuffer.bufferMemory);
  mDevice.freeMemory(mCameraDataBuffer.bufferMemory);
  mDevice.destroyBuffer(mCameraDataBuffer.buffer);

  mDevice.unmapMemory(mModelBuffer.bufferMemory);
  mDevice.freeMemory(mModelBuffer.bufferMemory);
  mDevice.destroyBuffer(mModelBuffer.buffer);
}
