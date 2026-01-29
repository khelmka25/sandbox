
#include "Graphics/Shader.h"

#include <glad/glad.h>

#include <filesystem>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <utility>

using namespace std::literals::string_view_literals;

namespace sb {
Shader::Shader(std::filesystem::path vertexPath, std::filesystem::path fragmentPath) {
  /*Vertex Shader*/

  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCreateShader.xhtml
  vertexShaderHandle = glCreateShader(GL_VERTEX_SHADER);

  std::string vertexShaderSource = Shader::loadShaderCode(vertexPath.string());
  const char* vertexShaderSourcePtr = vertexShaderSource.c_str();

  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glShaderSource.xhtml
  glShaderSource(vertexShaderHandle, 1, &vertexShaderSourcePtr, nullptr);

  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCompileShader.xhtml
  glCompileShader(vertexShaderHandle);

  // https://registry.khronos.org/OpenGL-Refpages/es2.0/xhtml/glGetShaderiv.xml
  GLint vertexStatus;
  glGetShaderiv(vertexShaderHandle, GL_COMPILE_STATUS, &vertexStatus);
  if (vertexStatus != GL_TRUE) {
    GLchar infoLog[1024];
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetShaderInfoLog.xhtml
    glGetShaderInfoLog(vertexShaderHandle, 1024, nullptr, infoLog);
    std::cout << infoLog << std::endl;
  }

  /*Fragment Shader*/

  fragmentShaderHandle = glCreateShader(GL_FRAGMENT_SHADER);

  std::string fragmentShaderSource = Shader::loadShaderCode(fragmentPath.string());
  const char* fragmentShaderSourcePtr = fragmentShaderSource.c_str();

  glShaderSource(fragmentShaderHandle, 1, &fragmentShaderSourcePtr, nullptr);
  glCompileShader(fragmentShaderHandle);

  GLint fragmentStatus;
  glGetShaderiv(fragmentShaderHandle, GL_COMPILE_STATUS, &fragmentStatus);
  if (fragmentStatus != GL_TRUE) {
    GLchar infoLog[1024];
    glGetShaderInfoLog(fragmentShaderHandle, 1024, nullptr, infoLog);
    std::cout << infoLog << std::endl;
  }

  // Create the shader program with Id
  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCreateProgram.xhtml
  programHandle = glCreateProgram();

  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glAttachShader.xhtml
  glAttachShader(programHandle, vertexShaderHandle);
  glAttachShader(programHandle, fragmentShaderHandle);

  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDeleteShader.xhtml
  glDeleteShader(vertexShaderHandle);
  glDeleteShader(fragmentShaderHandle);

  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glLinkProgram.xhtml
  glLinkProgram(programHandle);

  GLint shaderStatus;
  glGetProgramiv(programHandle, GL_LINK_STATUS, &shaderStatus);
  if (shaderStatus != GL_TRUE) {
    GLchar infoLog[1024];
    glGetProgramInfoLog(programHandle, 1024, nullptr, infoLog);
    std::cout << infoLog << std::endl;
  }

  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glUseProgram.xhtml
  glUseProgram(programHandle);
}

Shader::~Shader() {
  glUseProgram(0u);
  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDeleteProgram.xhtml
  glDeleteProgram(programHandle);
}

std::string Shader::loadShaderCode(std::string path) noexcept(false) {
  std::ifstream sourceCodeFile(path);
  if (sourceCodeFile.fail()) {
    std::cout << "Failed to open " << std::quoted(path) << std::endl;
    return {};
  }

  std::string line, source_code;
  while (std::getline(sourceCodeFile, line, '\n')) {
    source_code += (line + '\n');
  }

  return source_code;
}

void Shader::use() {
  glUseProgram(programHandle);
}

void Shader::disable() {
  glUseProgram(0u);
}

GLuint Shader::handle(void) {
  return programHandle;
}

int Shader::findUniformLocation(std::string_view name) {
  // try to find the uniform in the map
  if (uniformLocations.find(name) != uniformLocations.end()) {
    const auto location = uniformLocations.at(name);
    return location;
  }
  auto const location = glGetUniformLocation(this->programHandle, name.data());
  if (location < 0) {
    std::cout << "invalid uniform name: " << name << std::endl;
  }
  uniformLocations.insert({name, location});
  return location;
}

void Shader::setBool(std::string_view name, bool value) {
  const auto location = findUniformLocation(name);
  glUniform1i(location, (int)value);
}

void Shader::setInt(std::string_view name, int value) {
  const auto location = findUniformLocation(name);
  glUniform1i(location, value);
}

void Shader::setFloat(std::string_view name, float value) {
  const auto location = findUniformLocation(name);
  glUniform1f(location, value);
}

void Shader::setVec2(std::string_view name, const glm::vec2& value) {
  const auto location = findUniformLocation(name);
  glUniform2fv(location, 1, &value[0]);
}
void Shader::setVec2(std::string_view name, float x, float y) {
  const auto location = findUniformLocation(name);
  glUniform2f(location, x, y);
}

void Shader::setVec3(std::string_view name, const glm::vec3& value) {
  const auto location = findUniformLocation(name);
  glUniform3fv(location, 1, &value[0]);
}
void Shader::setVec3(std::string_view name, float x, float y, float z) {
  const auto location = findUniformLocation(name);
  glUniform3f(location, x, y, z);
}

void Shader::setVec4(std::string_view name, const glm::vec4& value) {
  const auto location = findUniformLocation(name);
  glUniform4fv(location, 1, &value[0]);
}
void Shader::setVec4(std::string_view name, float x, float y, float z, float w) {
  const auto location = findUniformLocation(name);
  glUniform4f(location, x, y, z, w);
}

void Shader::setMat2(std::string_view name, const glm::mat2& mat) {
  const auto location = findUniformLocation(name);
  glUniformMatrix2fv(location, 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(std::string_view name, const glm::mat3& mat) {
  const auto location = findUniformLocation(name);
  glUniformMatrix3fv(location, 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4(std::string_view name, const glm::mat4& mat) {
  const auto location = findUniformLocation(name);
  glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
}
}  // namespace sb