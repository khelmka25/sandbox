#include "Graphics/Context.h"

#include <stdexcept>
#include <iostream>

Context::Context(std::string_view title, glm::vec2 size) noexcept(false) {
  // Setup openGL
  if (glfwInit() != GLFW_TRUE) {
    throw std::runtime_error("Could not initialize glfw");
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create Window
  window = glfwCreateWindow(size.x, size.y, title.data(), nullptr, nullptr);
  if (window == nullptr) {
    glfwTerminate();
    throw std::runtime_error("Could not create glfw context");
  }

  // https://www.glfw.org/docs/latest/group__context.html#ga1c04dc242268f827290fe40aa1c91157
  glfwMakeContextCurrent(window);

  // Setup callbacks
  glfwSetFramebufferSizeCallback(
      window, [](GLFWwindow*, int newWidth, int newHeight) -> void {
        glViewport(0, 0, newWidth, newHeight);
      });

  glfwSetErrorCallback([](int error_code, const char* description) -> void {
    std::cout << "GLFW error " << error_code << ": " << description
              << std::endl;
  });

  // Load glad
  if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {
    glfwTerminate();
    throw std::runtime_error("Could not initialize glad");
  }
}

Context::~Context() noexcept {
  glfwDestroyWindow(window);
  glfwTerminate();
}