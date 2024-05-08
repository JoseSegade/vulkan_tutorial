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
  struct FinalizationChunk {
    vk::PhysicalDevice physicalDevice;
    vk::Device         device;
    vk::Queue          queue;
    vk::CommandBuffer  commandBuffer;
  };

 public:
  VertexMenagerie();
  ~VertexMenagerie();
  void init();
  void consume(
    vkMesh::MeshTypes type,
    const std::vector<float>& vertexData,
    const std::vector<Index>& indices);
  void finalize(const FinalizationChunk& input);
  const vkUtil::Buffer& getVertexBuffer();
  const vkUtil::Buffer& getIndexBuffer();
  uint32_t getOffset(vkMesh::MeshTypes type) const;
  uint32_t getSize(vkMesh::MeshTypes type) const;

 private:
  vkUtil::Buffer                                  mVertexBuffer;
  vkUtil::Buffer                                  mIndexBuffer;
  std::unordered_map<vkMesh::MeshTypes, uint32_t> mFirstIndices;
  std::unordered_map<vkMesh::MeshTypes, uint32_t> mIndexCounts;
  uint32_t                                        mIndexOffset = 0;
  vk::Device                                      mDevice;
  std::vector<float>                              mVertexLump;
  std::vector<Index>                              mIndexLump;
};

#endif  // INC_VERTEXMENAGERIE_H_
