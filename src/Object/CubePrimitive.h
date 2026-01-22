#pragma once

#include "Object/Object.h"

#include <glm/glm.hpp>

extern float cubeVertices[216];

class CubePrimitive : public Object {
 public:
  CubePrimitive() noexcept(true);
  ~CubePrimitive() noexcept(true);

  virtual void draw(class Shader* shader) override;

 protected:
  unsigned int vertexArrayHandle;
  unsigned int vertexBufferHandle;
};