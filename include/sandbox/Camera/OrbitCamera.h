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
class OrbitCamera : public Camera {
 public:
  // construct with position, radius
  OrbitCamera(glm::vec3 t_origin, float t_radius)
      : OrbitCamera(t_origin, glm::vec3(0.f, 1.f, 0.f), glm::vec3(1.f, 0.f, 0.f), t_radius, 0.1f, 0.f, 0.f) {}

  OrbitCamera(glm::vec3 t_origin, glm::vec3 t_upwards, glm::vec3 t_rightwards, float t_radius, float t_minRadius,
              float t_phi, float t_theta)
      : Camera(),
        origin(t_origin),
        upwards(t_upwards),
        rightwards(t_rightwards),
        radius(t_radius),
        minRadius(t_minRadius),
        phi(t_phi),
        theta(t_theta),
        positionSensitivity(0.5f),
        scrollSensitivity(0.5f) {
    // viewport(0, 1, 0, 1);
    // recompute();
  }

  virtual ~OrbitCamera() noexcept(true) override final = default;

  inline glm::vec3 position() override { return positionVector + origin; }

 public:
  // controller style: Fusion 360-esque
  // middle click hold + mouse movement orbits about origin
  // ctrl + middle click hold + mouse movement pans about viewpoint
  virtual void processEvent(const KeyboardEvent& e) noexcept override;
  virtual void processEvent(const MouseButtonEvent& e) noexcept override;
  virtual void processEvent(const MousePositionEvent& e) noexcept override;
  virtual void processEvent(const ScrollEvent& e) noexcept override;

 protected:
  void rotate(float dx, float dy);
  void translate(float dx, float dy);
  void zoom(float distance);

 protected:
  void recompute();

 public:
  // current origin, (may not be [0,0,0])
  glm::vec3 origin;

  glm::vec3 forwards;
  glm::vec3 upwards;
  glm::vec3 rightwards;

  // spherical coordinate system
  float radius;
  float minRadius;
  float theta = 0.f;
  float phi = 0.f;

  glm::vec3 positionVector;

  // constants
  const float positionSensitivity;
  const float scrollSensitivity;

  // state vars
  bool ctrlPressed = false;
  bool mouseMiddlePressed = false;
  
  float xposPrev;
  float yposPrev;
};
}  // namespace sb