#pragma once

#include <glm/vec3.hpp>

struct Material {
  Material() = default;
  glm::vec3 albedo = glm::vec3(1, 1, 1);
  float metallic = 0.1f;
  float roughnesss = 0.8f;
};