#include "Application/Window.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stb_image.h>

#include <iostream>
#include <queue>
#include <stdexcept>
#include <variant>
#include <filesystem>

#include "Application/Callbacks.h"
#include "Application/Event.h"

namespace sb::gfx {
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
  glfwSetFramebufferSizeCallback(window, &cb::glfwFramebufferSizeCallback);
  glfwSetErrorCallback(&cb::glfwErrorCallback);
  glfwSetKeyCallback(window, &cb::glfwKeyCallback);
  glfwSetMouseButtonCallback(window, &cb::glfwMouseButtonCallback);
  glfwSetCursorPosCallback(window, &cb::glfwCursorPosCallback);
  glfwSetScrollCallback(window, &cb::glfwScrollCallback);

  // Load glad
  if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {
    glfwTerminate();
    throw std::runtime_error("Could not initialize glad");
  }

  return window;
}

void setGLFWwindowIcon(GLFWwindow* window, std::filesystem::path filepath) {
  GLFWimage icon;
  int channels;
  icon.pixels = stbi_load(filepath.c_str(), &icon.width, &icon.height, &channels, STBI_rgb_alpha);
  glfwSetWindowIcon(window, 1, &icon);
  stbi_image_free(icon.pixels);
}

inline void destroyGLFWwindow(GLFWwindow* window) {
  glfwDestroyWindow(window);
  glfwTerminate();
}
}  // namespace sb::gfx