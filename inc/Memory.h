// Copyright (c) 2024 Meerkat
#ifndef INC_MEMORY_H_
#define INC_MEMORY_H_

#include "Common.h"

namespace vkUtil {

struct BufferInputChunk {
  size_t               size;
  vk::BufferUsageFlags usage;
  vk::Device           device;
  vk::PhysicalDevice   physicalDevice;
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

  uint32_t typeIndex = 0;
  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
    bool isSupported =
      static_cast<bool>(supportedMemoryIndices & (1 << i));

    bool isSufficient =
      (memoryProperties.memoryTypes[i].propertyFlags & requestedProperties)
      == requestedProperties;

    if (isSupported && isSufficient) {
      typeIndex = i;
      break;
    }
  }

  return typeIndex;
}

inline void allocateBufferMemory(Buffer* buffer,
                                 const BufferInputChunk& input) {
  vk::MemoryRequirements memoryRequirements =
    input.device.getBufferMemoryRequirements(buffer->buffer);

  vk::MemoryAllocateInfo allocInfo {};
  allocInfo.allocationSize = memoryRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryTypeIndex(
    input.physicalDevice, memoryRequirements.memoryTypeBits,
    vk::MemoryPropertyFlagBits::eHostVisible
    | vk::MemoryPropertyFlagBits::eHostCoherent);

  buffer->bufferMemory = input.device.allocateMemory(allocInfo);
  input.device.bindBufferMemory(buffer->buffer, buffer->bufferMemory, 0);
}

inline Buffer createBuffer(const BufferInputChunk& input) {
  vk::BufferCreateInfo bufferInfo {};
  bufferInfo.flags = vk::BufferCreateFlags();
  bufferInfo.size = input.size;
  bufferInfo.usage = input.usage;
  bufferInfo.sharingMode = vk::SharingMode::eExclusive;

  Buffer buffer {};
  buffer.buffer = input.device.createBuffer(bufferInfo);

  allocateBufferMemory(&buffer, input);

  return buffer;
}

}  // namespace vkUtil

#endif  // INC_MEMORY_H_
