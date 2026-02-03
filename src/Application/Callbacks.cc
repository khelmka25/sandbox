#include "Application/Event.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Data.h"

namespace sb::cb {
void glfwFramebufferSizeCallback(GLFWwindow*, int newWidth, int newHeight) {
  glViewport(0, 0, newWidth, newHeight);
  data::displayWidth = newWidth;
  data::displayHeight = newHeight;
}

void glfwErrorCallback(int error_code, const char* description) {
  std::cout << "GLFW error " << error_code << ": " << description << std::endl;
}

void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  KeyboardEvent event;
  event.key = key;
  event.scancode = scancode;
  event.action = action;
  event.mods = mods;
  eventQueue.emplace(EventType::kKeyboardEvent, event);
}

void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
  MouseButtonEvent event;
  event.button = button;
  event.action = action;
  event.mods = mods;
  eventQueue.emplace(EventType::kMouseButtonEvent, event);
}

void glfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
  MousePositionEvent event;
  event.xpos = static_cast<float>(xpos);
  event.ypos = static_cast<float>(ypos);
  eventQueue.emplace(EventType::kMousePositionEvent, event);
}

void glfwScrollCallback(GLFWwindow* window, double xoff, double yoff) {
  ScrollEvent event;
  event.dx = static_cast<float>(xoff);
  event.dy = static_cast<float>(yoff);
  eventQueue.emplace(EventType::kScrollEvent, event);
}
}  // namespace sb::cb