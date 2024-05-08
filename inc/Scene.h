// Copyright (c) 2024 Meerkat
#ifndef INC_SCENE_H_
#define INC_SCENE_H_

#include "Common.h"
#include "Mesh.h"
#include <unordered_map>
#include <vector>

class Scene {
 public:
  Scene();

  void init();

  std::unordered_map<vkMesh::MeshTypes, std::vector<glm::vec3>> positions;
};

#endif  // INC_SCENE_H_
