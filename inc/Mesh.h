// Copyright (c) 2024 Meerkat
#ifndef INC_MESH_H_
#define INC_MESH_H_

#include "Common.h"
#include <array>

namespace vkMesh {

enum class MeshTypes : uint32_t {
  TRIANGLE,
  SQUARE,
  STAR
};

inline vk::VertexInputBindingDescription getPosColorBindingDescription() {
  vk::VertexInputBindingDescription bindingDescription {};
  bindingDescription.binding = 0;
  bindingDescription.stride = 5 * sizeof(float);
  bindingDescription.inputRate = vk::VertexInputRate::eVertex;

  return bindingDescription;
}

inline std::array<vk::VertexInputAttributeDescription, 2>
getPosColorAttributeDescriptions() {
  std::array<vk::VertexInputAttributeDescription, 2> attributes {};

  // Pos
  attributes[0].binding = 0;
  attributes[0].location = 0;
  attributes[0].format = vk::Format::eR32G32Sfloat;
  attributes[0].offset = 0;

  // Color
  attributes[1].binding = 0;
  attributes[1].location = 1;
  attributes[1].format = vk::Format::eR32G32B32Sfloat;
  attributes[1].offset = 2 * sizeof(float);

  return attributes;
}

}  // namespace vkMesh

#endif  // INC_MESH_H_