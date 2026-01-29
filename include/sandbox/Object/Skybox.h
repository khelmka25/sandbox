#pragma once

#include <glad/glad.h>

#include <initializer_list>
#include <string_view>

#include "Graphics/Shader.h"

namespace sb {
extern float skyboxVertices[108];

class Skybox {
 public:
  Skybox() noexcept(true);
  ~Skybox() noexcept(true);

  void draw(class Shader* shader);

  unsigned cubemap;

 protected:
  unsigned vertexArrayHandle;
  unsigned vertexBufferHandle;
};
}  // namespace sb