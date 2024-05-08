// Copyright (c) 2024 Meerkat
#include "../inc/Scene.h"

Scene::Scene() {
}

void Scene::init() {
  positions.insert({ vkMesh::MeshTypes::GROUND, {} });
  positions.insert({ vkMesh::MeshTypes::GIRL,   {} });
  positions.insert({ vkMesh::MeshTypes::SKULL,  {} });

  positions[vkMesh::MeshTypes::GROUND]
    .push_back(glm::vec3(10.0f, 0.0f, 0.0f));
  positions[vkMesh::MeshTypes::GIRL]
    .push_back(glm::vec3(17.0f, 0.0f, 0.0f));
  positions[vkMesh::MeshTypes::SKULL]
    .push_back(glm::vec3(15.0f, -5.0f, 0.0f));
  positions[vkMesh::MeshTypes::SKULL]
    .push_back(glm::vec3(15.0f, 5.0f, 0.0f));
}
