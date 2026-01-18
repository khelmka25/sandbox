#pragma once

#include <glad/glad.h>

#include <string_view>

#include "Graphics/Context.h"
#include "Graphics/Shader.h"

class Application {
 public:
  Application();
  ~Application() noexcept;

 public:
  virtual void build(Context* ctx);
  virtual void draw(Context* ctx) noexcept(false);
  virtual void recomputeViewport(glm::vec2 size);
  virtual bool isOpen() noexcept;

  Context context;
  Shader shader;

  unsigned vertexBufferHandle;
  unsigned vertexArrayHandle;

  unsigned indexBufferHandle;
};