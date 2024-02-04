// Copyright (c) 2024 Meerkat
#ifndef INC_SCENE_H_
#define INC_SCENE_H_

#include "Common.h"

class Scene {
 public:
  Scene();

  void init();
  const std::vector<glm::vec3>& getTrianglePositions();
  const std::vector<glm::vec3>& getSquarePositions();
  const std::vector<glm::vec3>& getStarPositions();

 private:
  std::vector<glm::vec3> mTrianglePositions {};
  std::vector<glm::vec3> mSquarePositions {};
  std::vector<glm::vec3> mStarPositions {};
};

#endif  // INC_SCENE_H_
