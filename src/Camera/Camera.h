#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string_view>

#include "Application/Event.h"

class Camera {
 public:
  explicit Camera(std::string_view t_name, glm::vec3 t_origin, glm::vec3 t_upwards, glm::vec3 t_rightwards,
                  float t_radius, float t_minRadius, float t_phi, float t_theta)
      : name(t_name),
        origin(t_origin),
        upwards(t_upwards),
        rightwards(t_rightwards),
        radius(t_radius),
        minRadius(t_minRadius),
        phi(t_phi),
        theta(t_theta),
        positionSensitivity(0.5f),
        scrollSensitivity(0.5f) {
    recomputeInternals();
  }

  ~Camera() noexcept(true) = default;

  inline glm::vec3 position() { return positionVector + origin; }

  // returns the view matrix calculated using Euler Angles and the LookAt Matrix
  inline glm::mat4 view() { return viewMatrix; }

 public:
  // controller style: Fusion 360-esque
  // middle click hold + mouse movement orbits about origin
  // ctrl + middle click hold + mouse movement pans about viewpoint
  void handleKeyboardEvent(const KeyboardEvent& e);
  void handleMouseButtonEvent(const MouseButtonEvent& e);
  void handlePositionEvent(const MousePositionEvent& e);
  void handleScrollEvent(const ScrollEvent& e);

 protected:
  void rotateVertical(float angle_deg);
  void rotateHorizontal(float angle_deg);
  void zoom(float distance);
  void translateVertical(float distance);
  void translateHorizontal(float distance);

  void recomputeInternals();

 public:
  // name of the camera object
  std::string_view name;
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

  glm::mat4 viewMatrix;
  glm::vec3 positionVector;

  // constants
  const float positionSensitivity;
  const float scrollSensitivity;

  // state vars
  bool ctrlPressed = false;
  bool scrollPressed = false;
  float xposPrev;
  float yposPrev;
};
