#pragma once

#include <glad/glad.h>

#include <string_view>

#include "Application/Context.h"
#include "Graphics/Shader.h"
#include "Camera/Camera.h"
#include "Object/Skybox.h"
#include "Object/Object.h"
#include "Object/CubePrimitive.h"

class Application {
 public:
  Application(char** argv, int argc) noexcept(true);
  ~Application() noexcept(true);

 public:
  virtual void handleEvents();
  void handleKeyboardEvent(const KeyboardEvent& e);
  void handleMouseButtonEvent(const MouseButtonEvent& e);
  void handlePositionEvent(const MousePositionEvent& e);
  void handleScrollEvent(const ScrollEvent& e);
  virtual void draw() noexcept(false);
  virtual bool isOpen() noexcept;

  Context context;
  Camera camera;

  CubePrimitive cubePrimitive;

  std::array<CubePrimitive, 6ull> cubes;
  std::array<glm::vec3, 6ull> colors;

  std::vector<std::shared_ptr<Object>> objects;
  Skybox skybox;
  
  Shader objectShader;
  Shader skyboxShader;
};