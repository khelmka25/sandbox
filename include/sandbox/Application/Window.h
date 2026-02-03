#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string_view>
#include <filesystem>

namespace sb {
namespace gfx {
extern GLFWwindow* createGLFWwindow(std::string_view title, unsigned width, unsigned height) noexcept(false);
extern void setGLFWwindowIcon(GLFWwindow* window, std::filesystem::path filepath);
extern void destroyGLFWwindow(GLFWwindow* window);

}  // namespace gfx
}  // namespace sb