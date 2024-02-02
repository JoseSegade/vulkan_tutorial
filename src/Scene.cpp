// Copyright (c) 2024 Meerkat
#include "../inc/Scene.h"

Scene::Scene() {
}

void Scene::Init() {
  for (float x = -1.0f; x < 1.0f; x += 0.2f) {
    for (float y = -1.0f; y < 1.0f; y += 0.2f) {
      mTrianglePositions.push_back(glm::vec3(x, y, 0.0f));
    }
  }
}

const std::vector<glm::vec3>& Scene::GetTrianglePositions() {
  return mTrianglePositions;
}
