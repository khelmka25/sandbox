#pragma once

#include <glad/glad.h>

#include <initializer_list>
#include <string_view>

#include "Graphics/Shader.h"
#include "Graphics/Texture/CubeMap.h"

extern float skyboxVertices[108];

class Skybox {
 public:
  Skybox() noexcept(true);
  ~Skybox() noexcept(true);

  void draw(class Shader* shader);

  auto textureHandle() {
    return cubemap.textureHandle();
  }

 protected:
  CubeMap cubemap;
  unsigned vertexArrayHandle;
  unsigned vertexBufferHandle;
};