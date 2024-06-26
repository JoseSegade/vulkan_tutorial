// Copyright (c) 2024 Meerkat
#ifndef INC_MESH_H_
#define INC_MESH_H_

#include "Common.h"
#include <vector>

namespace vkMesh {

enum class MeshTypes : uint32_t {
  GROUND,
  GIRL,
  SKULL,
};

// 3 pos 3 color 2 texcoord 3 normal
static const uint32_t VERTEX_COMPONENTS = 11;

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
  attributes.resize(4);

  // Pos
  attributes[0].binding  = 0;
  attributes[0].location = 0;
  attributes[0].format   = vk::Format::eR32G32B32Sfloat;
  attributes[0].offset   = 0;

  // Color
  attributes[1].binding  = 0;
  attributes[1].location = 1;
  attributes[1].format   = vk::Format::eR32G32B32Sfloat;
  attributes[1].offset   = 3 * sizeof(float);

  // TexCoord
  attributes[2].binding  = 0;
  attributes[2].location = 2;
  attributes[2].format   = vk::Format::eR32G32Sfloat;
  attributes[2].offset   = 6 * sizeof(float);

  // Normal
  attributes[3].binding  = 0;
  attributes[3].location = 3;
  attributes[3].format   = vk::Format::eR32G32B32Sfloat;
  attributes[3].offset   = 8 * sizeof(float);

  return attributes;
}

}  // namespace vkMesh

#endif  // INC_MESH_H_
