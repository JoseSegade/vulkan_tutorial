// Copyright (c) 2024 Meerkat
#ifndef INC_FRAME_H_
#define INC_FRAME_H_

#include "Common.h"
#include "Memory.h"

namespace vkUtil {

struct UniformBufferObject {
  glm::mat4 view;
  glm::mat4 projection;
  glm::mat4 viewProjection;
};

struct SwapChainFrame {
  // Swapchain
  vk::Image           image;
  vk::ImageView       imageView;
  vk::Framebuffer     framebuffer;

  // Command
  vk::CommandBuffer   commandBuffer;

  // Sync
  vk::Semaphore       imageAvailable;
  vk::Semaphore       renderFinished;
  vk::Fence           inFlight;

  // Resources
  UniformBufferObject cameraData;
  Buffer              cameraDataBuffer;
  void*               cameraDataWriteLocation;

  // Resource descriptors
  vk::DescriptorBufferInfo uniformBufferDescriptor;
  vk::DescriptorSet descriptorSet;

  inline void make_ubo_resources(vk::PhysicalDevice physicalDevice,
                                 vk::Device device) {
    BufferInputChunk input {};
    input.physicalDevice   = physicalDevice;
    input.device           = device;
    input.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible
      | vk::MemoryPropertyFlagBits::eHostCoherent;
    input.size             = sizeof(UniformBufferObject);
    input.usage            = vk::BufferUsageFlagBits::eUniformBuffer;

    cameraDataBuffer = createBuffer(input);

    cameraDataWriteLocation = device.mapMemory(
      cameraDataBuffer.bufferMemory, 0, sizeof(UniformBufferObject));

    uniformBufferDescriptor.buffer = cameraDataBuffer.buffer;
    uniformBufferDescriptor.offset = 0;
    uniformBufferDescriptor.range = sizeof(UniformBufferObject);
  }

  inline void write_descriptor_set(vk::Device device) {
    vk::WriteDescriptorSet writeInfo {};
    writeInfo.dstSet = descriptorSet;
    writeInfo.dstBinding = 0;
    writeInfo.dstArrayElement = 0;
    writeInfo.descriptorCount = 1;
    writeInfo.descriptorType = vk::DescriptorType::eUniformBuffer;
    writeInfo.pBufferInfo = &uniformBufferDescriptor;

    device.updateDescriptorSets(writeInfo, nullptr);
  }
};

}  // namespace vkUtil

#endif  // INC_FRAME_H_
