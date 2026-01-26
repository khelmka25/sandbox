#pragma once

#include "Object/Object.h"

#include <glm/glm.hpp>

extern float cubeVertices[216];

class CubePrimitive : public Object {
 public:
  CubePrimitive(glm::vec3 t_center, glm::vec4 t_albedo) noexcept(true);
  ~CubePrimitive() noexcept(true);

  virtual void draw(class Shader* shader) override;

  glm::vec4 albedo;

 protected:
  unsigned int vertexArrayHandle;
  unsigned int vertexBufferHandle;
};