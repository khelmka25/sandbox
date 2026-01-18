#pragma once

#include <string_view>
#include <filesystem>
#include <unordered_map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
 public:
  // load from file
  Shader(std::filesystem::path vertexPath, std::filesystem::path fragmentPath);
  ~Shader();

  unsigned int getProgramHandle(void);

  static std::string loadShaderCode(std::string path) noexcept(false);

  void registerUniform(const std::string_view name);
  void unregisterUniform(const std::string_view name);

  void enable();
  void disable();
  
  void setBool(const std::string_view name, bool value);
  void setInt(const std::string_view name, int value);
  void setFloat(const std::string_view name, float value);
  void setMat4(const std::string_view name, glm::mat4 value);
  
 protected:
  int findUniformLocation(std::string_view name);

 protected:
  unsigned int shaderProgramHandle, vertexShaderHandle, fragmentShaderHandle;
  // map uniform names to their locations
  std::unordered_map<std::string_view, int> uniformLocations;
};