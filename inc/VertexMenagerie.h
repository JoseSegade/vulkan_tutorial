// Copyright (c) 2024 Meerkat
#ifndef INC_VERTEXMENAGERIE_H_
#define INC_VERTEXMENAGERIE_H_

#include "Common.h"
#include "Memory.h"
#include "Mesh.h"
#include <vector>
#include <unordered_map>

class VertexMenagerie {
 public:
  VertexMenagerie();
  ~VertexMenagerie();
  void init();
  void consume(vkMesh::MeshTypes type, const std::vector<float>& vertexData);
  void finalize(vk::PhysicalDevice physicalDevice, vk::Device device);
  const vkUtil::Buffer& getVertexBuffer();
  uint32_t getOffset(vkMesh::MeshTypes type) const;
  uint32_t getSize(vkMesh::MeshTypes type) const;

 private:
  vkUtil::Buffer mVertexBuffer;
  std::unordered_map<vkMesh::MeshTypes, uint32_t> mOffsets;
  std::unordered_map<vkMesh::MeshTypes, uint32_t> mSizes;
  uint32_t mOffset = 0;
  vk::Device mDevice;
  std::vector<float> mLump;
};

#endif  // INC_VERTEXMENAGERIE_H_
