#include "Camera/Camera.h"

#include <glm/gtc/quaternion.hpp>

#include <GLFW/glfw3.h>

void Camera::handleKeyboardEvent(const KeyboardEvent& e) {
  if (e.key == GLFW_KEY_LEFT_CONTROL) {
    if (e.action == GLFW_PRESS) {
      std::cout << "Ctrl Press" << std::endl;
      ctrlPressed = true;
    }
    if (e.action == GLFW_RELEASE) {
      std::cout << "Ctrl Release" << std::endl;
      ctrlPressed = false;
    }
  }

  if (e.key == GLFW_KEY_UP) {
    rotateAboutOrigin({0, +0.5f});
  }

  if (e.key == GLFW_KEY_DOWN) {
    rotateAboutOrigin({0, -0.5f});
  }

  if (e.key == GLFW_KEY_LEFT) {
    rotateAboutOrigin({+0.5f, 0});
  }

  if (e.key == GLFW_KEY_RIGHT) {
    rotateAboutOrigin({-0.5f, 0});
  }
}

void Camera::handleMouseButtonEvent(const MouseButtonEvent& e) {
  // lookfor: middle click with mods
  if (e.button == GLFW_MOUSE_BUTTON_MIDDLE) {
    if (e.action == GLFW_PRESS) {
      std::cout << "Scroll Press" << std::endl;
      scrollPressed = true;
    } else if (e.action == GLFW_RELEASE) {
      std::cout << "Scroll Release" << std::endl;
      scrollPressed = false;
    }
  }
}

void Camera::handlePositionEvent(const MousePositionEvent& e) {
  auto dx = e.xpos - xposPrev;
  auto dy = e.ypos - yposPrev;

  xposPrev = e.xpos;
  yposPrev = e.ypos;

  dx *= positionSensitivity;
  dy *= positionSensitivity;

  // if middle mouse pressed, rotate about origin
  // if ctrl + middle mouse pressed, pan about viewport
  if (scrollPressed) {
    if (ctrlPressed) {
      // pan about viewport
      panAboutViewport({dx * positionSensitivity, dy * positionSensitivity});
    } else {
      // // rotate about origin, pretty involved process...
      rotateAboutOrigin({dx, dy});
    }
  }
}

void Camera::handleScrollEvent(const ScrollEvent& e) {
  const auto f = e.dy * scrollSensitivity;
  zoomToOrigin(f);
}

void Camera::panAboutViewport(const glm::vec2 delta) {
  // goal: compute new right and up vectors at the given angles
  // then move the origin and position along these vectors

  // rotation matrix for these values
  glm::mat4 model(1);
  auto upVector = glm::vec3(0, 1, 0);
  model = glm::rotate(model, glm::radians(Yaw), upVector);
  // // we have pitch (up down spherically)
  auto pitchVector = glm::vec3(1, 0, 0);
  model = glm::rotate(model, glm::radians(Pitch), pitchVector);

  auto right4 = glm::vec4(1, 0, 0, 1);
  auto up4 = glm::vec4(0, 1, 0, 1);
  // apply the rotation matrix about
  right4 = model * right4;
  up4 = model * up4;
    
  auto right = glm::vec3(right4);
  auto up = glm::vec3(up4);

  // pan the camera by moving up and rightward along the vectors
  // also move the origin the same amount
  auto travelRightwards = -right * delta.x;
  auto travelUpwards = up * delta.y;

  auto offset = travelRightwards + travelUpwards;

  origin = origin + offset;
  // recompute position...
  rotateAboutOrigin({0, 0});
  std::cout << "origin" << origin.x << ',' << origin.y << ',' << origin.z << std::endl;
}

void Camera::rotateAboutOrigin(const glm::vec2 delta) {
  // rotate the camera about the origin
  // update the right, up, and forward vectors
  Pitch += -delta.y;
  Yaw += -delta.x;
  
  glm::mat4 model(1);
  // translate to global origin
  model = glm::translate(model, -origin);
  // yaw (left right spherically)
  auto upVector = glm::vec3(0, 1, 0);
  model = glm::rotate(model, glm::radians(Yaw), upVector);
  // // we have pitch (up down spherically)
  auto pitchVector = glm::vec3(1, 0, 0);
  model = glm::rotate(model, glm::radians(Pitch), pitchVector);
  // translate back to custom origin
  model = glm::translate(model, +origin);

  // update the position
  glm::vec4 pos4(origin.x, origin.y, origin.z + distance, 1.f);
  pos4 = model * pos4;

  position = glm::vec3(pos4);

  std::cout << position.x << ',' << position.y << ',' << position.z << std::endl;
}

void Camera::zoomToOrigin(float travel) {
  // move the position towards the origin up to a distance
  // of 1mm (0.001f), this function is 1/x behavior?
  // everything is unchanged except position
  distance = std::max(0.001f, distance - travel);
  // recompute position...
  rotateAboutOrigin({0, 0});
}