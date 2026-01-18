
#include <filesystem>
#include <fstream>
#include <iostream>
#include <utility>

#include "Graphics/Shader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

using namespace std::literals::string_view_literals;

Shader::Shader(std::filesystem::path vertexPath,
                           std::filesystem::path fragmentPath) {
  /*Vertex Shader*/

  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCreateShader.xhtml
  vertexShaderHandle = glCreateShader(GL_VERTEX_SHADER);

  std::string vertexShaderSource =
      Shader::loadShaderCode(vertexPath.string());
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

  std::string fragmentShaderSource =
      Shader::loadShaderCode(fragmentPath.string());
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
  shaderProgramHandle = glCreateProgram();

  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glAttachShader.xhtml
  glAttachShader(shaderProgramHandle, vertexShaderHandle);
  glAttachShader(shaderProgramHandle, fragmentShaderHandle);

  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDeleteShader.xhtml
  glDeleteShader(vertexShaderHandle);
  glDeleteShader(fragmentShaderHandle);

  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glLinkProgram.xhtml
  glLinkProgram(shaderProgramHandle);

  GLint shaderStatus;
  glGetProgramiv(shaderProgramHandle, GL_LINK_STATUS, &shaderStatus);
  if (shaderStatus != GL_TRUE) {
    GLchar infoLog[1024];
    glGetProgramInfoLog(shaderProgramHandle, 1024, nullptr, infoLog);
    std::cout << infoLog << std::endl;
  }

  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glUseProgram.xhtml
  glUseProgram(shaderProgramHandle);
}

Shader::~Shader() {
  glUseProgram(0u);
  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDeleteProgram.xhtml
  glDeleteProgram(shaderProgramHandle);
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

void Shader::enable() {
  glUseProgram(shaderProgramHandle);
}

void Shader::disable() {
  glUseProgram(0u);
}

GLuint Shader::getProgramHandle(void) {
  return shaderProgramHandle;
}

int Shader::findUniformLocation(std::string_view name) {
  // try to find the uniform in the map
  if (uniformLocations.find(name) != uniformLocations.end()) {
    const auto location = uniformLocations.at(name);
    return location;
  }
  auto const location =
      glGetUniformLocation(this->shaderProgramHandle, name.data());
  uniformLocations.insert({name, location});
  return location;
}

void Shader::setBool(const std::string_view name, bool value) {
  const auto location = findUniformLocation(name);
  glUniform1i(location, static_cast<int>(value));
}

void Shader::setInt(const std::string_view name, int value) {
  const auto location = findUniformLocation(name);
  glUniform1i(location, value);
}

void Shader::setFloat(const std::string_view name, float value) {
  const auto location = findUniformLocation(name);
  glUniform1f(location, value);
}

void Shader::setMat4(const std::string_view name, glm::mat4 value) {
  // void glUniformMatrix4fv( 	GLint location,
  // GLsizei count,
  // GLboolean transpose,
  // const GLfloat *value);
  const auto location = findUniformLocation(name);
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::registerUniform(
    const std::string_view name) noexcept(false) {
  // ensure the uniform is created only once
  if (uniformLocations.find(name) != uniformLocations.end()) {
    throw std::runtime_error("This uniform already exists");
  }
  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetUniformLocation.xhtml
  auto location = glGetUniformLocation(getProgramHandle(), name.data());
  if (location != -1) {
    uniformLocations.insert({name, location});
  } else {
    std::cout << "name: " << name << " does not exist" << std::endl;
    throw std::runtime_error("This uniform does not exist");
  }
}

void Shader::unregisterUniform(const std::string_view name) {
  if (uniformLocations.find(name) != uniformLocations.end()) {
    uniformLocations.erase(name);
  } else {
    throw std::runtime_error("This uniform is not currently registered");
  }
}