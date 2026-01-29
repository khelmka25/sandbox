#pragma once

#include <glm/vec3.hpp>

namespace sb {
struct Vertex {
  Vertex() = default;

  // position
  glm::vec3 Position; //  12 bytes
  // normal
  glm::vec3 Normal; // 12 bytes
  // texCoords
  glm::vec2 TexCoords; // 8 bytes
};
}  // namespace sb