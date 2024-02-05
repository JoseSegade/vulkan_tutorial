// Copyright (c) 2024 Meerkat
#include "../inc/VertexMenagerie.h"

VertexMenagerie::VertexMenagerie() {
}

VertexMenagerie::~VertexMenagerie() {
  mDevice.destroyBuffer(mVertexBuffer.buffer);
  mDevice.freeMemory(mVertexBuffer.bufferMemory);
}

void VertexMenagerie::consume(
  vkMesh::MeshTypes type,
  const std::vector<float>& vertexData) {
  for (float attribute : vertexData) {
    mLump.push_back(attribute);
  }

  uint32_t vertexCount =
    static_cast<uint32_t>(vertexData.size() / 5);

  mOffsets.insert(std::make_pair(type, mOffset));
  mSizes.insert(std::make_pair(type, vertexCount));

  mOffset += vertexCount;
}

void VertexMenagerie::finalize(const FinalizationChunk& input) {
  mDevice = input.device;

  vkUtil::BufferInputChunk inputChunk {};
  inputChunk.device           = input.device;
  inputChunk.physicalDevice   = input.physicalDevice;
  inputChunk.size             = sizeof(float) * mLump.size();
  inputChunk.usage            = vk::BufferUsageFlagBits::eTransferSrc;
  inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible
    | vk::MemoryPropertyFlagBits::eHostCoherent;

  vkUtil::Buffer stagingBuffer = vkUtil::createBuffer(inputChunk);

  void* memoryLocation = mDevice.mapMemory(stagingBuffer.bufferMemory,
                                           0, inputChunk.size);
  memcpy(memoryLocation, mLump.data(), inputChunk.size);
  mDevice.unmapMemory(stagingBuffer.bufferMemory);

  inputChunk.usage            = vk::BufferUsageFlagBits::eTransferDst
    | vk::BufferUsageFlagBits::eVertexBuffer;
  inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;

  mVertexBuffer = vkUtil::createBuffer(inputChunk);

  vkUtil::copyBuffer(&stagingBuffer, &mVertexBuffer, inputChunk.size,
                     input.queue, input.commandBuffer);

  mDevice.destroyBuffer(stagingBuffer.buffer);
  mDevice.freeMemory(stagingBuffer.bufferMemory);
}

const vkUtil::Buffer& VertexMenagerie::getVertexBuffer() {
  return mVertexBuffer;
}

uint32_t VertexMenagerie::getOffset(vkMesh::MeshTypes type) const {
  return mOffsets.at(type);
}

uint32_t VertexMenagerie::getSize(vkMesh::MeshTypes type) const {
  return mSizes.at(type);
}
