#ifndef INC_TEXTURE_H_
#define INC_TEXTURE_H_

#include "Common.h"

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

}

#endif  // INC_TEXTURE_H_
