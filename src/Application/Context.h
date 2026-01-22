#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <string_view>
#include <queue>
#include <Application/Event.h>

struct Context {
 public:
  Context(std::string_view title, glm::vec2 size) noexcept(false);
  ~Context() noexcept;

  GLFWwindow* window;

 public:
  static std::queue<Event> eventQueue;
};