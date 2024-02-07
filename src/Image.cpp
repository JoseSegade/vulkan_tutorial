// Copyright (c) 2024 Meerkat
#include "../inc/Image.h"
#include "../inc/Memory.h"
#include "../inc/Descriptors.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../ext/stb/stb_image.h"

vkImage::Texture::Texture() {
}

vkImage::Texture::~Texture() {
  mDevice.freeMemory(mImageMemory);
  mDevice.destroyImage(mImage);
  mDevice.destroyImageView(mImageView);
  mDevice.destroySampler(mSampler);
}

void vkImage::Texture::init(const TextureInputChunk& input) {
  mDevice         = input.device;
  mPhysicalDevice = input.physicalDevice;
  mFilename       = input.filename;
  mCommandBuffer  = input.commandBuffer;
  mQueue          = input.queue;
  mLayout         = input.layout;
  mDescriptorPool = input.descriptorPool;

  load();

  ImageInputChunk imageInput {};
  imageInput.device           = mDevice;
  imageInput.physicalDevice   = mPhysicalDevice;
  imageInput.width            = mWidth;
  imageInput.height           = mHeight;
  imageInput.tiling           = vk::ImageTiling::eOptimal;
  imageInput.usage            = vk::ImageUsageFlagBits::eTransferDst
    | vk::ImageUsageFlagBits::eSampled;
  imageInput.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;

  mImage = make_image(imageInput);
  mImageMemory = make_image_memory(imageInput, mImage);

  populate();

  free(mPixels);

  make_view();
  make_sampler();
  make_descriptor_set();
}

void vkImage::Texture::use(vk::CommandBuffer commandBuffer,
                           vk::PipelineLayout pipelineLayout) {
  commandBuffer.bindDescriptorSets(
    vk::PipelineBindPoint::eGraphics, pipelineLayout,
    1, mDescriptorSet, nullptr);
}

void vkImage::Texture::load() {
  int width = 0;
  int height = 0;
  int channels = 0;
  mPixels   = stbi_load(mFilename, &width, &height, &channels, STBI_rgb_alpha);
  mWidth    = static_cast<uint32_t>(width);
  mHeight   = static_cast<uint32_t>(height);
  mChannels = static_cast<uint32_t>(channels);

  if (!mPixels) {
    printf("Error while loading image: %s\n",
           mFilename);
  }
}

void vkImage::Texture::populate() {
  vkUtil::BufferInputChunk input {};
  input.device           = mDevice;
  input.physicalDevice   = mPhysicalDevice;
  input.memoryProperties = vk::MemoryPropertyFlagBits::eHostCoherent
    | vk::MemoryPropertyFlagBits::eHostVisible;
  input.usage            = vk::BufferUsageFlagBits::eTransferDst;
  input.size             = mWidth * mHeight * 4;

  vkUtil::Buffer stagingBuffer = vkUtil::createBuffer(input);

  void* writeLocation = mDevice.mapMemory(stagingBuffer.bufferMemory,
                                          0, input.size);
  memcpy(writeLocation, mPixels, input.size);
  mDevice.unmapMemory(stagingBuffer.bufferMemory);

  ImageLayoutTransitionJob transitionJob {};
  transitionJob.commandBuffer = mCommandBuffer;
  transitionJob.queue         = mQueue;
  transitionJob.image         = mImage;
  transitionJob.oldLayout     = vk::ImageLayout::eUndefined;
  transitionJob.newLayout     = vk::ImageLayout::eTransferDstOptimal;
  transition_image_layout(transitionJob);

  BufferImageCopyJob copyJob {};
  copyJob.commandBuffer = mCommandBuffer;
  copyJob.queue         = mQueue;
  copyJob.srcBuffer     = stagingBuffer.buffer;
  copyJob.dstImage      = mImage;
  copyJob.width         = mWidth;
  copyJob.height        = mHeight;
  copy_buffer_to_image(copyJob);

  transitionJob.oldLayout = vk::ImageLayout::eTransferDstOptimal;
  transitionJob.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
  transition_image_layout(transitionJob);

  mDevice.freeMemory(stagingBuffer.bufferMemory);
  mDevice.destroyBuffer(stagingBuffer.buffer);
}

void vkImage::Texture::make_view() {
  mImageView = make_image_view(mDevice, mImage, vk::Format::eR8G8B8A8Unorm);
}

void vkImage::Texture::make_sampler() {
  vk::SamplerCreateInfo samplerInfo {};
  samplerInfo.flags                   = vk::SamplerCreateFlags();
  samplerInfo.minFilter               = vk::Filter::eNearest;
  samplerInfo.magFilter               = vk::Filter::eLinear;
  samplerInfo.addressModeU            = vk::SamplerAddressMode::eRepeat;
  samplerInfo.addressModeV            = vk::SamplerAddressMode::eRepeat;
  samplerInfo.addressModeW            = vk::SamplerAddressMode::eRepeat;
  samplerInfo.anisotropyEnable        = false;
  samplerInfo.maxAnisotropy           = 1.0f;
  samplerInfo.borderColor             = vk::BorderColor::eIntOpaqueBlack;
  samplerInfo.unnormalizedCoordinates = false;
  samplerInfo.compareEnable           = false;
  samplerInfo.compareOp               = vk::CompareOp::eAlways;
  samplerInfo.mipmapMode              = vk::SamplerMipmapMode::eLinear;
  samplerInfo.mipLodBias              = 0.0f;
  samplerInfo.minLod                  = 0.0f;
  samplerInfo.maxLod                  = 0.0f;

  try {
    mSampler = mDevice.createSampler(samplerInfo);
  } catch (vk::SystemError err) {
    printf("Error while creating image sampler. Image: %s, Error: %s\n",
           mFilename, err.what());
  }
}

void vkImage::Texture::make_descriptor_set() {
  bool debug = true;
  mDescriptorSet  = vkInit::allocate_descriptor_set(
    mDevice, mDescriptorPool, mLayout, debug);

  vk::DescriptorImageInfo imageDescriptor {};
  imageDescriptor.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
  imageDescriptor.imageView   = mImageView;
  imageDescriptor.sampler     = mSampler;

  vk::WriteDescriptorSet descriptorWrite {};
  descriptorWrite.dstSet          = mDescriptorSet;
  descriptorWrite.dstBinding      = 0;
  descriptorWrite.dstArrayElement = 0;
  descriptorWrite.descriptorType  = vk::DescriptorType::eCombinedImageSampler;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.pImageInfo      = &imageDescriptor;

  mDevice.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
}

vk::Image vkImage::make_image(const ImageInputChunk& input) {
  vk::ImageCreateInfo imageInfo {};
  imageInfo.flags         = vk::ImageCreateFlagBits();
  imageInfo.imageType     = vk::ImageType::e2D;
  imageInfo.extent        = vk::Extent3D(input.width, input.height, 1);
  imageInfo.mipLevels     = 1;
  imageInfo.arrayLayers   = 1;
  imageInfo.format        = vk::Format::eR8G8B8A8Unorm;
  imageInfo.tiling        = input.tiling;
  imageInfo.initialLayout = vk::ImageLayout::eUndefined;
  imageInfo.usage         = input.usage;
  imageInfo.sharingMode   = vk::SharingMode::eExclusive;
  imageInfo.samples       = vk::SampleCountFlagBits::e1;

  vk::Image image {};
  try {
    image = input.device.createImage(imageInfo);
  } catch (vk::SystemError err) {
    printf("Error while creating image. Error: %s\n",
           err.what());
  }

  return image;
}

vk::DeviceMemory vkImage::make_image_memory(const ImageInputChunk& input,
                                            vk::Image image) {
  vk::MemoryRequirements requirements =
    input.device.getImageMemoryRequirements(image);

  vk::MemoryAllocateInfo allocation {};
  allocation.allocationSize  = requirements.size;
  allocation.memoryTypeIndex = vkUtil::findMemoryTypeIndex(
    input.physicalDevice, requirements.memoryTypeBits, input.memoryProperties);

  vk::DeviceMemory res {};
  try {
    res = input.device.allocateMemory(allocation);
    input.device.bindImageMemory(image, res, 0);
  } catch (vk::SystemError err) {
    printf("Error while allocating memory for image. Error: %s\n",
           err.what());
  }

  return res;
}

void vkImage::transition_image_layout(const ImageLayoutTransitionJob& job) {
  vkUtil::start_job(job.commandBuffer);

  vk::ImageSubresourceRange access {};
  access.aspectMask     = vk::ImageAspectFlagBits::eColor;
  access.baseMipLevel   = 0;
  access.levelCount     = 1;
  access.baseArrayLayer = 0;
  access.layerCount     = 1;

  vk::ImageMemoryBarrier barrier {};
  barrier.oldLayout = job.oldLayout;
  barrier.newLayout = job.newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = job.image;
  barrier.subresourceRange = access;

  vk::PipelineStageFlags srcStage {};
  vk::PipelineStageFlags dstStage {};
  if (job.oldLayout == vk::ImageLayout::eUndefined) {
    barrier.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

    srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
    dstStage = vk::PipelineStageFlagBits::eTransfer;
  } else {
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    srcStage = vk::PipelineStageFlagBits::eTransfer;
    dstStage = vk::PipelineStageFlagBits::eFragmentShader;
  }

  job.commandBuffer.pipelineBarrier(srcStage, dstStage,
                                    vk::DependencyFlags(),
                                    nullptr, nullptr,
                                    barrier);

  vkUtil::end_job(job.commandBuffer, job.queue);
}

void vkImage::copy_buffer_to_image(const BufferImageCopyJob& job) {
  vkUtil::start_job(job.commandBuffer);

  vk::ImageSubresourceLayers access {};
  access.aspectMask     = vk::ImageAspectFlagBits::eColor;
  access.mipLevel       = 0;
  access.baseArrayLayer = 0;
  access.layerCount     = 1;

  vk::BufferImageCopy copy {};
  copy.bufferOffset      = 0;
  copy.bufferRowLength   = 0;
  copy.bufferImageHeight = 0;
  copy.imageSubresource  = access;
  copy.imageOffset       = vk::Offset3D(0, 0, 0);
  copy.imageExtent       = vk::Extent3D(job.width, job.height, 1);

  job.commandBuffer.copyBufferToImage(
    job.srcBuffer, job.dstImage, vk::ImageLayout::eTransferDstOptimal, copy);

  vkUtil::end_job(job.commandBuffer, job.queue);
}

vk::ImageView vkImage::make_image_view(
  vk::Device device, vk::Image image, vk::Format format) {
  vk::ImageViewCreateInfo createInfo{};
  createInfo.image        = image;
  createInfo.viewType     = vk::ImageViewType::e2D;
  createInfo.components.r = vk::ComponentSwizzle::eIdentity;
  createInfo.components.g = vk::ComponentSwizzle::eIdentity;
  createInfo.components.b = vk::ComponentSwizzle::eIdentity;
  createInfo.components.a = vk::ComponentSwizzle::eIdentity;
  createInfo.subresourceRange.aspectMask     =
    vk::ImageAspectFlagBits::eColor;
  createInfo.subresourceRange.baseMipLevel   = 0;
  createInfo.subresourceRange.levelCount     = 1;
  createInfo.subresourceRange.baseArrayLayer = 0;
  createInfo.subresourceRange.layerCount     = 1;
  createInfo.format = format;

  vk::ImageView res = device.createImageView(createInfo);
  return res;
}
