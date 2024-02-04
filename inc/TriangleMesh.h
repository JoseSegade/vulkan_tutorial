// Copyright (c) 2023 Meerkat
#ifndef INC_TRIANGLEMESH_H_
#define INC_TRIANGLEMESH_H_

#include "Common.h"
#include "Memory.h"

class TriangleMesh {
 public:
  TriangleMesh();
  ~TriangleMesh();

  void init(vk::PhysicalDevice physicalDevice, vk::Device device);
  const vkUtil::Buffer& getVertexBuffer();

 private:
  vk::Device     mDevice;
  vkUtil::Buffer mVertexBuffer;
};

#endif  // INC_TRIANGLEMESH_H_
