// Copyright (c) 2024 Meerkat
#include "../inc/TriangleMesh.h"

TriangleMesh::TriangleMesh() {
}

TriangleMesh::~TriangleMesh() {
  mDevice.destroyBuffer(mVertexBuffer.buffer);
  mDevice.freeMemory(mVertexBuffer.bufferMemory);
}

void TriangleMesh::init(vk::PhysicalDevice physicalDevice, vk::Device device) {
  mDevice = device;

  std::vector<float> vertices = { {
     0.00f, -0.05f, 0.0f, 1.0f, 0.0f,
     0.05f,  0.05f, 0.0f, 1.0f, 0.0f,
    -0.05f,  0.05f, 0.0f, 1.0f, 0.0f } };

  vkUtil::BufferInput inputChunk;
  inputChunk.device         = device;
  inputChunk.physicalDevice = physicalDevice;
  inputChunk.size           = sizeof(float) * vertices.size();
  inputChunk.usage          = vk::BufferUsageFlagBits::eVertexBuffer;

  mVertexBuffer = vkUtil::createBuffer(inputChunk);

  void* memoryLocation = device.mapMemory(mVertexBuffer.bufferMemory,
                                          0, inputChunk.size);

  memcpy(memoryLocation, vertices.data(), inputChunk.size);
  device.unmapMemory(mVertexBuffer.bufferMemory);
}

const vkUtil::Buffer& TriangleMesh::getVertexBuffer() {
  return mVertexBuffer;
}
