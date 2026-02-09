#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Object/Object.h"

// extern float cubeVertices[216];

namespace sb {
class CubePrimitive : public Object {
 public:
  CubePrimitive(glm::vec3 t_center, glm::vec4 t_albedo) noexcept(true);
  ~CubePrimitive() noexcept(true);

  virtual void draw(class Shader* shader) override;

  glm::vec4 albedo;

 protected:
  static glm::vec3 vertices[24];
  static glm::vec3 normals[24];
  static glm::vec2 uvs[24];
  static unsigned indices[36];

 protected:
  unsigned vbo, vao, ebo;
};
}  // namespace sb