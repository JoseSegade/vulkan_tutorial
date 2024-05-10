#include "../inc/Cubemap.h"
#include "../inc/Image.h"
#include "../inc/Descriptors.h"
#include "../inc/Memory.h"
#include "../ext/stb/stb_image.h"

vkImage::CubeMap::CubeMap() {
}

vkImage::CubeMap::~CubeMap() {
  mDevice.freeMemory(mImageMemory);
  mDevice.destroyImage(mImage);
  mDevice.destroyImageView(mImageView);
  mDevice.destroySampler(mSampler);
}

void vkImage::CubeMap::init(const CubeMapInputChunk& input) {
  mDevice         = input.device;
  mPhysicalDevice = input.physicalDevice;
  mFilenames      = input.filenames;
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
  imageInput.arraySize        = ARRAY_SIZE;
  imageInput.format           = vk::Format::eR8G8B8A8Unorm;
  imageInput.tiling           = vk::ImageTiling::eOptimal;
  imageInput.usage            = vk::ImageUsageFlagBits::eTransferDst
    | vk::ImageUsageFlagBits::eSampled;
  imageInput.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
  imageInput.flags            = vk::ImageCreateFlagBits::eCubeCompatible;

  mImage = make_image(imageInput);
  mImageMemory = make_image_memory(imageInput, mImage);

  populate();

  for (uint32_t i = 0; i < ARRAY_SIZE; ++i) {
    free(mPixels[i]);
  }

  make_view();
  make_sampler();
  make_descriptor_set();
}

void vkImage::CubeMap::use(vk::CommandBuffer commandBuffer,
                           vk::PipelineLayout pipelineLayout) {
  commandBuffer.bindDescriptorSets(
    vk::PipelineBindPoint::eGraphics,
    pipelineLayout,
    1,
    mDescriptorSet,
    nullptr
  );
}

void vkImage::CubeMap::load() {
  int pw = 0;
  int ph = 0;
  int pc = 0;
  for (uint32_t i = 0; i < ARRAY_SIZE; ++i) {
    int width = 0;
    int height = 0;
    int channels = 0;
    mPixels[i] = stbi_load(
      mFilenames.at(i),
      &width,
      &height,
      &channels,
      STBI_rgb_alpha
    );
    if (mPixels[i] == nullptr) {
      printf("Error while loading image: %s\n",
             mFilenames[i]);
      return;
    }
    if (i != 0 && (pw != width || ph != height || pc != channels)) {
      printf("Error: all cubemap images must be the same size");
      return;
    }
    pw = width;
    ph = height;
    pc = channels;
  }
  mWidth    = static_cast<uint32_t>(pw);
  mHeight   = static_cast<uint32_t>(ph);
  mChannels = static_cast<uint32_t>(pc);
}

void vkImage::CubeMap::populate() {
  const size_t image_size = static_cast<size_t>(mWidth * mHeight * 4);

  vkUtil::BufferInputChunk input {};
  input.device           = mDevice;
  input.physicalDevice   = mPhysicalDevice;
  input.memoryProperties = vk::MemoryPropertyFlagBits::eHostCoherent
    | vk::MemoryPropertyFlagBits::eHostVisible;
  input.usage            = vk::BufferUsageFlagBits::eTransferSrc;
  input.size             = image_size * ARRAY_SIZE;

  vkUtil::Buffer stagingBuffer = vkUtil::createBuffer(input);

  for (uint32_t i = 0; i < ARRAY_SIZE; ++i) {
    void* writeLocation = mDevice.mapMemory(
      stagingBuffer.bufferMemory,
      i * image_size,
      image_size
    );
    memcpy(writeLocation, mPixels[i], image_size);
    mDevice.unmapMemory(stagingBuffer.bufferMemory);
  }

  ImageLayoutTransitionJob transitionJob {};
  transitionJob.commandBuffer = mCommandBuffer;
  transitionJob.queue         = mQueue;
  transitionJob.image         = mImage;
  transitionJob.oldLayout     = vk::ImageLayout::eUndefined;
  transitionJob.newLayout     = vk::ImageLayout::eTransferDstOptimal;
  transitionJob.arraySize     = ARRAY_SIZE;
  transition_image_layout(transitionJob);

  BufferImageCopyJob copyJob {};
  copyJob.commandBuffer = mCommandBuffer;
  copyJob.queue         = mQueue;
  copyJob.srcBuffer     = stagingBuffer.buffer;
  copyJob.dstImage      = mImage;
  copyJob.width         = mWidth;
  copyJob.height        = mHeight;
  copyJob.arraySize     = ARRAY_SIZE;
  copy_buffer_to_image(copyJob);

  transitionJob.oldLayout = vk::ImageLayout::eTransferDstOptimal;
  transitionJob.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
  transition_image_layout(transitionJob);

  mDevice.freeMemory(stagingBuffer.bufferMemory);
  mDevice.destroyBuffer(stagingBuffer.buffer);
}

void vkImage::CubeMap::make_view() {
  mImageView = make_image_view(
    mDevice,
    mImage,
    vk::Format::eR8G8B8A8Unorm,
    vk::ImageAspectFlagBits::eColor,
    vk::ImageViewType::eCube,
    ARRAY_SIZE
  );
}

void vkImage::CubeMap::make_sampler() {
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
    printf("Error while creating image sampler. Error: %s\n",
            err.what());
  }
}

void vkImage::CubeMap::make_descriptor_set() {
  bool debug = true;
  mDescriptorSet  = vkInit::allocate_descriptor_set(
    mDevice,
    mDescriptorPool,
    mLayout,
    debug
  );

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
