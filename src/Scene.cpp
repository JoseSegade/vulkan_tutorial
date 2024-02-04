// Copyright (c) 2024 Meerkat
#include "../inc/Scene.h"

Scene::Scene() {
}

void Scene::init() {
  float x = -0.6f;
  for (float y = -1.0f; y < 1.0f; y += 0.2f) {
    mTrianglePositions.push_back(glm::vec3(x, y, 0.0f));
  }

  x = 0.0f;
  for (float y = -1.0f; y < 1.0f; y += 0.2f) {
    mSquarePositions.push_back(glm::vec3(x, y, 0.0f));
  }

  x = 0.6f;
  for (float y = -1.0f; y < 1.0f; y += 0.2f) {
    mStarPositions.push_back(glm::vec3(x, y, 0.0f));
  }
}

const std::vector<glm::vec3>& Scene::getTrianglePositions() {
  return mTrianglePositions;
}

const std::vector<glm::vec3>& Scene::getSquarePositions() {
  return mSquarePositions;
}

const std::vector<glm::vec3>& Scene::getStarPositions() {
  return mStarPositions;
}
