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

void VertexMenagerie::finalize(
  vk::PhysicalDevice physicalDevice,
  vk::Device device) {
  mDevice = device;

  vkUtil::BufferInputChunk inputChunk {};
  inputChunk.device = device;
  inputChunk.physicalDevice = physicalDevice;
  inputChunk.size = sizeof(float) * mLump.size();
  inputChunk.usage = vk::BufferUsageFlagBits::eVertexBuffer;

  mVertexBuffer = vkUtil::createBuffer(inputChunk);

  void* memoryLocation = mDevice.mapMemory(mVertexBuffer.bufferMemory,
                                           0, inputChunk.size);
  memcpy(memoryLocation, mLump.data(), inputChunk.size);
  mDevice.unmapMemory(mVertexBuffer.bufferMemory);
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
