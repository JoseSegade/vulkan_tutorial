// Copyright (c) 2024 Meerkat
#ifndef INC_SCENE_H_
#define INC_SCENE_H_

#include "Common.h"
#include <vector>

class Scene {
 public:
  Scene();

  void Init();
  const std::vector<glm::vec3>& GetTrianglePositions();

 private:
  std::vector<glm::vec3> mTrianglePositions {};
};

#endif  // INC_SCENE_H_
