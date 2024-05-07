// Copyright (c) 2024 Meerkat
#ifndef INC_IMAGE_H_
#define INC_IMAGE_H_

#include "Common.h"
#include <vector>

namespace vkImage {

struct TextureInputChunk {
  vk::Device              device;
  vk::PhysicalDevice      physicalDevice;
  const char*             filename;
  vk::CommandBuffer       commandBuffer;
  vk::Queue               queue;
  vk::DescriptorSetLayout layout;
  vk::DescriptorPool      descriptorPool;
};

struct ImageInputChunk {
  vk::Device              device;
  vk::PhysicalDevice      physicalDevice;
  uint32_t                width;
  uint32_t                height;
  vk::ImageTiling         tiling;
  vk::ImageUsageFlags     usage;
  vk::MemoryPropertyFlags memoryProperties;
  vk::Format              format;
};

struct ImageLayoutTransitionJob {
  vk::CommandBuffer commandBuffer;
  vk::Queue         queue;
  vk::Image         image;
  vk::ImageLayout   oldLayout;
  vk::ImageLayout   newLayout;
};

struct BufferImageCopyJob {
  vk::CommandBuffer commandBuffer;
  vk::Queue         queue;
  vk::Buffer        srcBuffer;
  vk::Image         dstImage;
  uint32_t          width;
  uint32_t          height;
};

class Texture {
 public:
  Texture();
  ~Texture();

  void init(const TextureInputChunk& input);
  void use(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout);

 private:
  void load();
  void populate();
  void make_view();
  void make_sampler();
  void make_descriptor_set();

 private:
  using stbi_uc = unsigned char;
  uint32_t                mWidth    = 0;
  uint32_t                mHeight   = 0;
  uint32_t                mChannels = 0;
  vk::Device              mDevice;
  vk::PhysicalDevice      mPhysicalDevice;
  const char*             mFilename;
  stbi_uc*                mPixels;

  // Resources
  vk::Image               mImage;
  vk::DeviceMemory        mImageMemory;
  vk::ImageView           mImageView;
  vk::Sampler             mSampler;

  // Resource descriptors
  vk::DescriptorSetLayout mLayout;
  vk::DescriptorSet       mDescriptorSet;
  vk::DescriptorPool      mDescriptorPool;

  vk::CommandBuffer       mCommandBuffer;
  vk::Queue               mQueue;
};

vk::Image make_image(const ImageInputChunk& input);
vk::DeviceMemory make_image_memory(
  const ImageInputChunk& input, vk::Image image);
void transition_image_layout(const ImageLayoutTransitionJob& job);
void copy_buffer_to_image(const BufferImageCopyJob& job);
vk::ImageView make_image_view(
  vk::Device device, vk::Image image, vk::Format format,
  vk::ImageAspectFlags aspectFlags);
vk::Format find_supported_format(
  vk::PhysicalDevice physicalDevice,
  const std::vector<vk::Format>& candidates,
  vk::ImageTiling tiling, vk::FormatFeatureFlags features);

}  // namespace vkImage

#endif  // INC_IMAGE_H_
