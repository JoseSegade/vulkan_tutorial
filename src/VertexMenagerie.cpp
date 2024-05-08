// Copyright (c) 2024 Meerkat
#include "../inc/VertexMenagerie.h"

VertexMenagerie::VertexMenagerie() {
}

VertexMenagerie::~VertexMenagerie() {
  mDevice.destroyBuffer(mVertexBuffer.buffer);
  mDevice.freeMemory(mVertexBuffer.bufferMemory);

  mDevice.destroyBuffer(mIndexBuffer.buffer);
  mDevice.freeMemory(mIndexBuffer.bufferMemory);
}

void VertexMenagerie::consume(
  vkMesh::MeshTypes type,
  const std::vector<float>& vertexData,
  const std::vector<Index>& indexData
) {
  uint32_t vertexCount =
    static_cast<uint32_t>(vertexData.size() / vkMesh::VERTEX_COMPONENTS);
  uint32_t indexCount =
    static_cast<uint32_t>(indexData.size());
  uint32_t lastIndex =
    static_cast<uint32_t>(mIndexLump.size());

  mFirstIndices.insert(std::make_pair(type, lastIndex));
  mIndexCounts.insert(std::make_pair(type, indexCount));

  for (const float& attribute : vertexData) {
    mVertexLump.push_back(attribute);
  }
  for (const Index& index : indexData) {
    mIndexLump.push_back(index + mIndexOffset);
  }

  mIndexOffset += vertexCount;
}

void VertexMenagerie::finalize(const FinalizationChunk& input) {
  mDevice = input.device;

  {
    vkUtil::BufferInputChunk inputChunk {};
    inputChunk.device           = input.device;
    inputChunk.physicalDevice   = input.physicalDevice;
    inputChunk.size             = sizeof(float) * mVertexLump.size();
    inputChunk.usage            = vk::BufferUsageFlagBits::eTransferSrc;
    inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible
      | vk::MemoryPropertyFlagBits::eHostCoherent;

    vkUtil::Buffer stagingBuffer = vkUtil::createBuffer(inputChunk);

    void* memoryLocation = mDevice.mapMemory(stagingBuffer.bufferMemory,
                                             0, inputChunk.size);
    memcpy(memoryLocation, mVertexLump.data(), inputChunk.size);
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

  {
    vkUtil::BufferInputChunk inputChunk {};
    inputChunk.device           = input.device;
    inputChunk.physicalDevice   = input.physicalDevice;
    inputChunk.size             = sizeof(Index) * mIndexLump.size();
    inputChunk.usage            = vk::BufferUsageFlagBits::eTransferSrc;
    inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible
      | vk::MemoryPropertyFlagBits::eHostCoherent;

    vkUtil::Buffer stagingBuffer = vkUtil::createBuffer(inputChunk);

    void* memoryLocation = mDevice.mapMemory(stagingBuffer.bufferMemory,
                                             0, inputChunk.size);
    memcpy(memoryLocation, mIndexLump.data(), inputChunk.size);
    mDevice.unmapMemory(stagingBuffer.bufferMemory);


    inputChunk.usage            = vk::BufferUsageFlagBits::eTransferDst
      | vk::BufferUsageFlagBits::eIndexBuffer;
    inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;

    mIndexBuffer = vkUtil::createBuffer(inputChunk);

    vkUtil::copyBuffer(&stagingBuffer, &mIndexBuffer, inputChunk.size,
                       input.queue, input.commandBuffer);

    mDevice.destroyBuffer(stagingBuffer.buffer);
    mDevice.freeMemory(stagingBuffer.bufferMemory);
  }

  mVertexLump.clear();
}

const vkUtil::Buffer& VertexMenagerie::getVertexBuffer() {
  return mVertexBuffer;
}

const vkUtil::Buffer& VertexMenagerie::getIndexBuffer() {
  return mIndexBuffer;
}

uint32_t VertexMenagerie::getOffset(vkMesh::MeshTypes type) const {
  return mFirstIndices.at(type);
}

uint32_t VertexMenagerie::getSize(vkMesh::MeshTypes type) const {
  return mIndexCounts.at(type);
}
