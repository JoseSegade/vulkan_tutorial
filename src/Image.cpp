// Copyright (c) 2024 Meerkat
#include "../inc/Image.h"
#include "../inc/Memory.h"

vk::Image vkImage::make_image(const ImageInputChunk& input) {
  vk::ImageCreateInfo imageInfo {};
  imageInfo.flags         = vk::ImageCreateFlagBits() | input.flags;
  imageInfo.imageType     = vk::ImageType::e2D;
  imageInfo.extent        = vk::Extent3D(input.width, input.height, 1);
  imageInfo.mipLevels     = 1;
  imageInfo.arrayLayers   = input.arraySize;
  imageInfo.format        = input.format;
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
  access.layerCount     = job.arraySize;

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
  access.layerCount     = job.arraySize;

  vk::BufferImageCopy copy {};
  copy.bufferOffset      = 0;
  copy.bufferRowLength   = 0;
  copy.bufferImageHeight = 0;
  copy.imageSubresource  = access;
  copy.imageOffset       = vk::Offset3D(0, 0, 0);
  copy.imageExtent       = vk::Extent3D(job.width, job.height, 1);

  job.commandBuffer.copyBufferToImage(
    job.srcBuffer,
    job.dstImage,
    vk::ImageLayout::eTransferDstOptimal,
    copy
  );

  vkUtil::end_job(job.commandBuffer, job.queue);
}

vk::ImageView vkImage::make_image_view(
  vk::Device device,
  vk::Image image,
  vk::Format format,
  vk::ImageAspectFlags aspectFlags,
  vk::ImageViewType type,
  uint32_t arraySize
) {
  vk::ImageViewCreateInfo createInfo{};
  createInfo.image        = image;
  createInfo.viewType     = type;
  createInfo.components.r = vk::ComponentSwizzle::eIdentity;
  createInfo.components.g = vk::ComponentSwizzle::eIdentity;
  createInfo.components.b = vk::ComponentSwizzle::eIdentity;
  createInfo.components.a = vk::ComponentSwizzle::eIdentity;
  createInfo.subresourceRange.aspectMask     = aspectFlags;
  createInfo.subresourceRange.baseMipLevel   = 0;
  createInfo.subresourceRange.levelCount     = 1;
  createInfo.subresourceRange.baseArrayLayer = 0;
  createInfo.subresourceRange.layerCount     = arraySize;
  createInfo.format = format;

  vk::ImageView res = device.createImageView(createInfo);
  return res;
}

vk::Format vkImage::find_supported_format(
  vk::PhysicalDevice physicalDevice,
  const std::vector<vk::Format>& candidates,
  vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
  for (const vk::Format& format : candidates) {
    vk::FormatProperties properties =
      physicalDevice.getFormatProperties(format);

    if (tiling == vk::ImageTiling::eLinear
        && (properties.linearTilingFeatures & features) == features) {
      return format;
    }

    if (tiling == vk::ImageTiling::eOptimal
        && (properties.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  printf("Error: Unable to find suitable format.\n");
  throw std::runtime_error("");
}
