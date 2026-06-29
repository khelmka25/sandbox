#pragma once

#include <GLFW/glfw3.h>

namespace sb::cb {
extern void glfwFramebufferSizeCallback(GLFWwindow*, int newWidth, int newHeight);
extern void glfwErrorCallback(int error_code, const char* description);
extern void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
extern void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
extern void glfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos);
extern void glfwScrollCallback(GLFWwindow* window, double xoff, double yoff);
extern void glfwDropCallback(GLFWwindow* window, int path_count, const char* paths[]);
};