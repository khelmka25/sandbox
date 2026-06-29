#pragma once

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <iostream>
#include <string_view>

#include "Application/Event.h"
#include "Camera/Camera.h"

namespace sb {
class ScrollCamera : public Camera {
 public:
  // scroll camera with position and zoom
  explicit ScrollCamera(glm::vec3 t_origin, float t_zoom)
      : ScrollCamera(t_origin, glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), t_zoom, 0.1f, 0.f, 0.f) {}
      
  ScrollCamera(glm::vec3 t_origin, glm::vec3 t_upwards, glm::vec3 t_rightwards, float t_radius, float t_minRadius,
               float t_phi, float t_theta)
      : Camera(),
        origin(t_origin),
        upwards(t_upwards),
        rightwards(t_rightwards),
        radius(t_radius),
        minRadius(t_minRadius),
        maxRadius(t_minRadius + 10.f),
        positionSensitivity(1.f),
        scrollSensitivity(1.f) {
    recompute();
  }

  virtual ~ScrollCamera() noexcept(true) override final = default;

 public:
  inline glm::vec3 position() { return origin; }

  virtual void processEvent(const KeyboardEvent& e) noexcept override;
  virtual void processEvent(const MouseButtonEvent& e) noexcept override;
  virtual void processEvent(const MousePositionEvent& e) noexcept override;
  virtual void processEvent(const ScrollEvent& e) noexcept override;

 protected:
  void zoom(float dr);
  void translate(float dx, float dy);
  void recompute();

 public:
  // current origin, (may not be [0,0,0])
  glm::vec3 origin;

  glm::vec3 forwards;
  glm::vec3 upwards;
  glm::vec3 rightwards;

  glm::vec3 positionVector;
  glm::mat4 orbitModel;

  float radius;
  float minRadius;
  float maxRadius;

  // constants
  const float positionSensitivity;
  const float scrollSensitivity;

  // state vars
  bool mouseLeftPressed = false;
  bool mouseMiddlePressed = false;

  glm::vec2 prevCursorPos;
};
}  // namespace sb