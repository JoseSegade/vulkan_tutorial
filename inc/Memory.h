// Copyright (c) 2024 Meerkat
#ifndef INC_MEMORY_H_
#define INC_MEMORY_H_

#include "Common.h"
#include "SingleTimeCommands.h"

namespace vkUtil {

struct BufferInputChunk {
  size_t                  size;
  vk::BufferUsageFlags    usage;
  vk::Device              device;
  vk::PhysicalDevice      physicalDevice;
  vk::MemoryPropertyFlags memoryProperties;
};

struct Buffer {
  vk::Buffer       buffer;
  vk::DeviceMemory bufferMemory;
};

inline uint32_t findMemoryTypeIndex(
  vk::PhysicalDevice physicalDevice, uint32_t supportedMemoryIndices,
  vk::MemoryPropertyFlags requestedProperties) {
  vk::PhysicalDeviceMemoryProperties memoryProperties =
    physicalDevice.getMemoryProperties();

  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
    bool isSupported =
      static_cast<bool>(supportedMemoryIndices & (1 << i));

    bool isSufficient =
      (memoryProperties.memoryTypes[i].propertyFlags & requestedProperties)
      == requestedProperties;

    if (isSupported && isSufficient) {
      return i;
    }
  }

  printf("Error type index not found\n");
  throw std::runtime_error("");
}

inline void allocateBufferMemory(Buffer* buffer,
                                 const BufferInputChunk& input) {
  vk::MemoryRequirements memoryRequirements =
    input.device.getBufferMemoryRequirements(buffer->buffer);

  vk::MemoryAllocateInfo allocInfo {};
  allocInfo.allocationSize  = memoryRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryTypeIndex(
    input.physicalDevice, memoryRequirements.memoryTypeBits,
    input.memoryProperties);

  buffer->bufferMemory = input.device.allocateMemory(allocInfo);
  input.device.bindBufferMemory(buffer->buffer, buffer->bufferMemory, 0);
}

inline Buffer createBuffer(const BufferInputChunk& input) {
  vk::BufferCreateInfo bufferInfo {};
  bufferInfo.flags       = vk::BufferCreateFlags();
  bufferInfo.size        = input.size;
  bufferInfo.usage       = input.usage;
  bufferInfo.sharingMode = vk::SharingMode::eExclusive;

  Buffer buffer {};
  buffer.buffer = input.device.createBuffer(bufferInfo);

  allocateBufferMemory(&buffer, input);

  return buffer;
}

inline void copyBuffer(const Buffer* srcBuffer, Buffer* dstBuffer,
                       vk::DeviceSize size, vk::Queue queue,
                       vk::CommandBuffer commandBuffer) {
  vkUtil::start_job(commandBuffer);

  vk::BufferCopy copyRegion{};
  copyRegion.srcOffset = 0;
  copyRegion.dstOffset = 0;
  copyRegion.size      = size;
  commandBuffer.copyBuffer(srcBuffer->buffer, dstBuffer->buffer,
                           1, &copyRegion);

  vkUtil::end_job(commandBuffer, queue);
}

}  // namespace vkUtil

#endif  // INC_MEMORY_H_
