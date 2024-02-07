// Copyright (c) 2024 Meerkat
#ifndef INC_FRAME_H_
#define INC_FRAME_H_

#include "Common.h"
#include "Memory.h"
#include <vector>

namespace vkUtil {

struct UniformBufferObject {
  glm::mat4 view;
  glm::mat4 projection;
  glm::mat4 viewProjection;
};

struct SwapChainFrame {
  // Swapchain
  vk::Image                image;
  vk::ImageView            imageView;
  vk::Framebuffer          framebuffer;

  // Command
  vk::CommandBuffer        commandBuffer;

  // Sync
  vk::Semaphore            imageAvailable;
  vk::Semaphore            renderFinished;
  vk::Fence                inFlight;

  // Resources
  UniformBufferObject      cameraData;
  Buffer                   cameraDataBuffer;
  void*                    cameraDataWriteLocation;
  std::vector<glm::mat4>   modelTransforms;
  Buffer                   modelBuffer;
  void*                    modelBufferWriteLocation;

  // Resource descriptors
  vk::DescriptorBufferInfo uniformBufferDescriptor;
  vk::DescriptorBufferInfo modelBufferDescriptor;
  vk::DescriptorSet        descriptorSet;

  inline void make_descriptor_resources(
    vk::PhysicalDevice physicalDevice, vk::Device device) {
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

    input.size             = 1024 * sizeof(glm::mat4);
    input.usage            = vk::BufferUsageFlagBits::eStorageBuffer;

    modelBuffer = createBuffer(input);

    modelBufferWriteLocation = device.mapMemory(
      modelBuffer.bufferMemory, 0, 1024 * sizeof(glm::mat4));

    modelTransforms.reserve(1024);
    for (uint32_t i = 0; i < 1024; ++i) {
      modelTransforms.push_back(glm::mat4(1.0f));
    }

    uniformBufferDescriptor.buffer = cameraDataBuffer.buffer;
    uniformBufferDescriptor.offset = 0;
    uniformBufferDescriptor.range  = sizeof(UniformBufferObject);

    modelBufferDescriptor.buffer = modelBuffer.buffer;
    modelBufferDescriptor.offset = 0;
    modelBufferDescriptor.range  = 1024 * sizeof(glm::mat4);
  }

  inline void write_descriptor_set(vk::Device device) {
    vk::WriteDescriptorSet writeInfo {};
    writeInfo.dstSet          = descriptorSet;
    writeInfo.dstBinding      = 0;
    writeInfo.dstArrayElement = 0;
    writeInfo.descriptorCount = 1;
    writeInfo.descriptorType  = vk::DescriptorType::eUniformBuffer;
    writeInfo.pBufferInfo     = &uniformBufferDescriptor;

    device.updateDescriptorSets(writeInfo, nullptr);

    vk::WriteDescriptorSet writeInfo2 {};
    writeInfo2.dstSet          = descriptorSet;
    writeInfo2.dstBinding      = 1;
    writeInfo2.dstArrayElement = 0;
    writeInfo2.descriptorCount = 1;
    writeInfo2.descriptorType  = vk::DescriptorType::eStorageBuffer;
    writeInfo2.pBufferInfo     = &modelBufferDescriptor;

    device.updateDescriptorSets(writeInfo2, nullptr);
  }
};

}  // namespace vkUtil

#endif  // INC_FRAME_H_
