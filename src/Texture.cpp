#include "../inc/Texture.h"
#include "../inc/Image.h"
#include "../inc/Descriptors.h"
#include "../inc/Memory.h"
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "../ext/stb/stb_image.h"
#endif  // STB_IMAGE_IMPLEMENTATION

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
  imageInput.arraySize        = 1;
  imageInput.format           = vk::Format::eR8G8B8A8Unorm;
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
    vk::PipelineBindPoint::eGraphics,
    pipelineLayout,
    1,
    mDescriptorSet,
    nullptr
  );
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
  input.usage            = vk::BufferUsageFlagBits::eTransferSrc;
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
  transitionJob.arraySize     = 1;
  transition_image_layout(transitionJob);

  BufferImageCopyJob copyJob {};
  copyJob.commandBuffer = mCommandBuffer;
  copyJob.queue         = mQueue;
  copyJob.srcBuffer     = stagingBuffer.buffer;
  copyJob.dstImage      = mImage;
  copyJob.width         = mWidth;
  copyJob.height        = mHeight;
  copyJob.arraySize     = 1;
  copy_buffer_to_image(copyJob);

  transitionJob.oldLayout = vk::ImageLayout::eTransferDstOptimal;
  transitionJob.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
  transition_image_layout(transitionJob);

  mDevice.freeMemory(stagingBuffer.bufferMemory);
  mDevice.destroyBuffer(stagingBuffer.buffer);
}

void vkImage::Texture::make_view() {
  mImageView = make_image_view(
    mDevice,
    mImage,
    vk::Format::eR8G8B8A8Unorm,
    vk::ImageAspectFlagBits::eColor,
    vk::ImageViewType::e2D,
    1
  );
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
