#pragma once

#include <filesystem>
#include <string_view>
#include <unordered_map>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

namespace sb {
class Shader {
 public:
  // load from file
  Shader(std::string_view t_name, std::filesystem::path vertexPath, std::filesystem::path fragmentPath);
  ~Shader();

  unsigned int handle(void);

  static std::string loadShaderCode(std::string path) noexcept(false);

  void use();
  void disable();

  void setBool(std::string_view name, bool value);
  void setInt(std::string_view name, int value);
  void setFloat(std::string_view name, float value);
  void setVec2(std::string_view name, const glm::vec2& value);
  void setVec2(std::string_view name, float x, float y);
  void setVec3(std::string_view name, const glm::vec3& value);
  void setVec3(std::string_view name, float x, float y, float z);
  void setVec4(std::string_view name, const glm::vec4& value);
  void setVec4(std::string_view name, float x, float y, float z, float w);
  void setMat2(std::string_view name, const glm::mat2& mat);
  void setMat3(std::string_view name, const glm::mat3& mat);
  void setMat4(std::string_view name, const glm::mat4& mat);

 protected:
  int findUniformLocation(std::string_view name);

 protected:
  unsigned int programHandle, vertexShaderHandle, fragmentShaderHandle;
  // map uniform names to their locations
  std::unordered_map<std::string_view, int> uniformLocations;

  std::string_view name;
};
}  // namespace sb