// Copyright (c) 2024 Meerkat
#ifndef INC_MESH_H_
#define INC_MESH_H_

#include "Common.h"
#include <vector>

namespace vkMesh {

enum class MeshTypes : uint32_t {
  TRIANGLE,
  SQUARE,
  STAR
};

// 2 pos (x, y) 3 color (r, g, b) 2 texcoord (u, v)
static const uint32_t VERTEX_COMPONENTS = 7;

inline vk::VertexInputBindingDescription getPosColorBindingDescription() {
  vk::VertexInputBindingDescription bindingDescription {};
  bindingDescription.binding   = 0;
  bindingDescription.stride    = VERTEX_COMPONENTS * sizeof(float);
  bindingDescription.inputRate = vk::VertexInputRate::eVertex;

  return bindingDescription;
}

inline std::vector<vk::VertexInputAttributeDescription>
getPosColorAttributeDescriptions() {
  std::vector<vk::VertexInputAttributeDescription> attributes {};
  attributes.resize(3);

  // Pos
  attributes[0].binding  = 0;
  attributes[0].location = 0;
  attributes[0].format   = vk::Format::eR32G32Sfloat;
  attributes[0].offset   = 0;

  // Color
  attributes[1].binding  = 0;
  attributes[1].location = 1;
  attributes[1].format   = vk::Format::eR32G32B32Sfloat;
  attributes[1].offset   = 2 * sizeof(float);

  // TexCoord
  attributes[2].binding  = 0;
  attributes[2].location = 2;
  attributes[2].format   = vk::Format::eR32G32Sfloat;
  attributes[2].offset   = 5 * sizeof(float);

  return attributes;
}

}  // namespace vkMesh

#endif  // INC_MESH_H_
