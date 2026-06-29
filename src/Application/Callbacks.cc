#include "Application/Event.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Data.h"

namespace sb::cb {
void glfwFramebufferSizeCallback(GLFWwindow*, int width, int height) {
  ViewportEvent event;
  event.width = width;
  event.height = height;
  eventQueue.emplace(EventType::kViewportEvent, event);
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

void glfwEnterCallback(GLFWwindow* window, int entered) {
  // eventQueue.emplace(EventType::kCursorEnterEvent, std::in_place_type<int>, entered);
  eventQueue.emplace(EventType::kCursorEnterEvent, std::in_place_type<int>, entered);
}

void glfwDropCallback(GLFWwindow* window, int path_count, const char* paths[]) {
  DropEvent event;
  for (int i = 0; i < path_count; i++) {
    event.paths.emplace_back(paths[i]);
  }
  eventQueue.emplace(EventType::kDropEvent, std::move(event));
}

}  // namespace sb::cb