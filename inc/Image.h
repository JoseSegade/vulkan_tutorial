// Copyright (c) 2024 Meerkat
#ifndef INC_IMAGE_H_
#define INC_IMAGE_H_

#include "Common.h"
#include <vector>

namespace vkImage {

struct ImageInputChunk {
  vk::Device              device;
  vk::PhysicalDevice      physicalDevice;
  uint32_t                width;
  uint32_t                height;
  vk::ImageTiling         tiling;
  vk::ImageUsageFlags     usage;
  vk::MemoryPropertyFlags memoryProperties;
  vk::Format              format;
  uint32_t                arraySize;
  vk::ImageCreateFlags    flags;
};

struct ImageLayoutTransitionJob {
  vk::CommandBuffer commandBuffer;
  vk::Queue         queue;
  vk::Image         image;
  vk::ImageLayout   oldLayout;
  vk::ImageLayout   newLayout;
  uint32_t          arraySize;
};

struct BufferImageCopyJob {
  vk::CommandBuffer commandBuffer;
  vk::Queue         queue;
  vk::Buffer        srcBuffer;
  vk::Image         dstImage;
  uint32_t          width;
  uint32_t          height;
  uint32_t          arraySize;
};


vk::Image make_image(const ImageInputChunk& input);
vk::DeviceMemory make_image_memory(
  const ImageInputChunk& input,
  vk::Image image
);
void transition_image_layout(const ImageLayoutTransitionJob& job);
void copy_buffer_to_image(const BufferImageCopyJob& job);
vk::ImageView make_image_view(
  vk::Device device,
  vk::Image image,
  vk::Format format,
  vk::ImageAspectFlags aspectFlags,
  vk::ImageViewType type,
  uint32_t arraySize
);
vk::Format find_supported_format(
  vk::PhysicalDevice physicalDevice,
  const std::vector<vk::Format>& candidates,
  vk::ImageTiling tiling,
  vk::FormatFeatureFlags features
);

}  // namespace vkImage

#endif  // INC_IMAGE_H_
