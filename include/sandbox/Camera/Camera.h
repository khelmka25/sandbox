#pragma once

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <iostream>
#include <string_view>

#include "Application/Event.h"

namespace sb {
class Camera {
 public:
  Camera() : positionSensitivity(1.f), scrollSensitivity(1.f) {};
  virtual ~Camera() noexcept(true) = default;

 public:
  virtual glm::vec3 position() = 0;

  // returns the view matrix calculated using Euler Angles and the LookAt Matrix
  inline const glm::mat4& view() { return viewMatrix; }
  inline const glm::mat4& proj() { return projMatrix; }

  inline void viewport(float t_left, float t_right, float t_bottom, float t_top) {
    vpLeft = t_left;
    vpRight = t_right;
    vpBottom = t_bottom;
    vpTop = t_top;
  }

  virtual void processEvent(const KeyboardEvent& e) noexcept = 0;
  virtual void processEvent(const MouseButtonEvent& e) noexcept = 0;
  virtual void processEvent(const MousePositionEvent& e) noexcept = 0;
  virtual void processEvent(const ScrollEvent& e) noexcept = 0;

 protected:
  glm::mat4 viewMatrix = glm::mat4(1.f);
  glm::mat4 projMatrix = glm::mat4(1.f);

  float vpLeft, vpRight;
  float vpBottom, vpTop;

  // constants
  const float positionSensitivity;
  const float scrollSensitivity;
};
}  // namespace sb