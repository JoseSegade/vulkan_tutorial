#ifndef INC_CUBEMAP_H_
#define INC_CUBEMAP_H_

#include "Common.h"

namespace vkImage {

struct CubeMapInputChunk {
  vk::Device               device;
  vk::PhysicalDevice       physicalDevice;
  std::vector<const char*> filenames;
  vk::CommandBuffer        commandBuffer;
  vk::Queue                queue;
  vk::DescriptorSetLayout  layout;
  vk::DescriptorPool       descriptorPool;
};

class CubeMap {
 public:
  const static uint32_t ARRAY_SIZE = 6;
  CubeMap();
  ~CubeMap();

  void init(const CubeMapInputChunk& input);
  void use(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout);

 private:
  void load();
  void populate();
  void make_view();
  void make_sampler();
  void make_descriptor_set();

 private:
  using stbi_uc = unsigned char;
  uint32_t                 mWidth    = 0;
  uint32_t                 mHeight   = 0;
  uint32_t                 mChannels = 0;
  vk::Device               mDevice;
  vk::PhysicalDevice       mPhysicalDevice;
  std::vector<const char*> mFilenames;
  stbi_uc*                 mPixels[6];

  // Resources
  vk::Image                mImage;
  vk::DeviceMemory         mImageMemory;
  vk::ImageView            mImageView;
  vk::Sampler              mSampler;

  // Resource descriptors
  vk::DescriptorSetLayout  mLayout;
  vk::DescriptorSet        mDescriptorSet;
  vk::DescriptorPool       mDescriptorPool;

  vk::CommandBuffer        mCommandBuffer;
  vk::Queue                mQueue;
};

}

#endif  // INC_CUBEMAP_H_
