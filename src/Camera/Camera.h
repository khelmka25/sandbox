#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string_view>

#include "Application/Event.h"

class Camera {
 public:
  explicit Camera(std::string_view t_name, glm::vec3 t_position, glm::vec3 t_origin)
      : name(t_name), position(t_position), origin(t_origin), positionSensitivity(0.5f), scrollSensitivity(0.5f) {
        distance = 3;
      }

    ~Camera() noexcept(true) = default;

  // returns the view matrix calculated using Euler Angles and the LookAt Matrix
  inline glm::mat4 view() { 
    // translation
    glm::mat4 model(1);
    model = glm::translate(model, position - origin);
    // define your up vector
    glm::vec3 upVector = glm::vec3(0, 1, 0);
    // rotate around to a given bearing: yaw
    glm::mat4 camera = glm::rotate(model, glm::radians(yaw_deg), upVector);
    // Define the 'look up' axis, should be orthogonal to the up axis
    glm::vec3 pitchVector = glm::vec3(1, 0, 0);
    // rotate around to the required head tilt: pitch
    camera = glm::rotate(camera, glm::radians(pitch_deg), pitchVector);
    camera = glm::translate(camera, origin);

    // now get the view matrix by taking the camera inverse
    glm::mat4 view = glm::inverse(camera);

    return view;
  }

 public:
  // controller style: Fusion 360-esque
  // middle click hold + mouse movement orbits about origin
  // ctrl + middle click hold + mouse movement pans about viewpoint
  void handleKeyboardEvent(const KeyboardEvent& e);
  void handleMouseButtonEvent(const MouseButtonEvent& e);
  void handlePositionEvent(const MousePositionEvent& e);
  void handleScrollEvent(const ScrollEvent& e);

 protected:
  // pan about viewport, recomputes origin
  void panAboutViewport(const glm::vec2 delta);

  // rotate about origin, recomputes position and angles
  void rotateAboutOrigin(const glm::vec2 delta);

  // move inwards
  void zoomToOrigin(float distance);
 
 public:
  // name of the camera object
  std::string_view name;
  // current position
  glm::vec3 position;
  // current origin, (may not be [0,0,0])
  glm::vec3 origin;

  float distance;

  // constants
  const float positionSensitivity;
  const float scrollSensitivity;

  // state vars
  bool ctrlPressed = false;
  bool scrollPressed = false;
  float xposPrev;
  float yposPrev;

  // euler Angles (degrees)
  float yaw_deg = 0.f;
  float pitch_deg = 0.f;
};
