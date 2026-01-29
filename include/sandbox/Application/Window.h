#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <queue>
#include <stdexcept>
#include <variant>

#include "Application/Event.h"

namespace gfx {
GLFWwindow* createGLFWwindow(std::string_view title, unsigned width, unsigned height) noexcept(false) {
  // Setup openGL
  if (glfwInit() != GLFW_TRUE) {
    throw std::runtime_error("Could not initialize glfw");
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  // Create Window
  GLFWwindow* window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
  if (window == nullptr) {
    glfwTerminate();
    throw std::runtime_error("Could not create glfw context");
  }

  // https://www.glfw.org/docs/latest/group__context.html#ga1c04dc242268f827290fe40aa1c91157
  glfwMakeContextCurrent(window);

  // Setup callbacks
  glfwSetFramebufferSizeCallback(
      window, [](GLFWwindow*, int newWidth, int newHeight) -> void { glViewport(0, 0, newWidth, newHeight); });

  glfwSetErrorCallback([](int error_code, const char* description) -> void {
    std::cout << "GLFW error " << error_code << ": " << description << std::endl;
  });

  glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
    KeyboardEvent event;
    event.key = key;
    event.scancode = scancode;
    event.action = action;
    event.mods = mods;
    eventQueue.emplace(EventType::kKeyboardEvent, event);
  });

  glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
    MouseButtonEvent event;
    event.button = button;
    event.action = action;
    event.mods = mods;
    eventQueue.emplace(EventType::kMouseButtonEvent, event);
  });

  glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
    MousePositionEvent event;
    event.xpos = static_cast<float>(xpos);
    event.ypos = static_cast<float>(ypos);
    eventQueue.emplace(EventType::kMousePositionEvent, event);
  });

  glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
    ScrollEvent event;
    event.dx = static_cast<float>(xoffset);
    event.dy = static_cast<float>(yoffset);
    eventQueue.emplace(EventType::kScrollEvent, event);
  });

  // Load glad
  if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {
    glfwTerminate();
    throw std::runtime_error("Could not initialize glad");
  }

  return window;
}

inline void destroyGLFWwindow(GLFWwindow* window) {
  glfwDestroyWindow(window);
  glfwTerminate();
}

}  // namespace gfx