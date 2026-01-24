#include "Camera/Camera.h"

#include <GLFW/glfw3.h>

#include <glm/gtc/quaternion.hpp>

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
    rotateVertical(+0.5f);
  }

  if (e.key == GLFW_KEY_DOWN) {
    rotateVertical(-0.5f);
  }

  if (e.key == GLFW_KEY_LEFT) {
    rotateHorizontal(+0.5f);
  }

  if (e.key == GLFW_KEY_RIGHT) {
    rotateHorizontal(-0.5f);
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

  // if middle mouse pressed, rotate about origin
  // if ctrl + middle mouse pressed, pan about viewport
  if (scrollPressed) {
    if (ctrlPressed) {
      // pan about viewport
      translateVertical(dy * 0.01f);
      translateHorizontal(dx * 0.01f);
      recomputeInternals();
    } else {
      // // rotate about origin, pretty involved process...
      rotateVertical(dy * 0.5f);
      rotateHorizontal(dx * 0.5f);
      recomputeInternals();
    }
  }
}

void Camera::handleScrollEvent(const ScrollEvent& e) {
  zoom(e.dy * scrollSensitivity);
  recomputeInternals();
}

void Camera::recomputeInternals() {
  // Calculate sines / cosines of angles
  const auto sineTheta = std::sin(glm::radians(theta));
  const auto cosineTheta = std::cos(glm::radians(theta));
  const auto sinePhi = std::sin(glm::radians(phi));
  const auto cosinePhi = std::cos(glm::radians(phi));

  // Calculate the position vector
  forwards.x = cosinePhi * cosineTheta;
  forwards.y = sinePhi;
  forwards.z = cosinePhi * sineTheta;

  // Calculate the rightwards vector
  const auto worldUp = glm::vec3(0, 1, 0);
  rightwards = glm::normalize(glm::cross(forwards, worldUp));

  // Calculate the upwards vector
  upwards = glm::normalize(glm::cross(rightwards, forwards));

  // Calculate the position vector
  positionVector = forwards * radius;

  // Calculate the view matrix
  viewMatrix = glm::lookAt(positionVector + origin, origin, upwards);
}

void Camera::rotateVertical(float angle_deg) {
  phi = std::clamp(phi + angle_deg, -89.0f, 89.0f);
}

void Camera::rotateHorizontal(float angle_deg) {
  theta += angle_deg;
}

void Camera::zoom(float distance) {
  radius = std::max(minRadius, radius - distance);
}

void Camera::translateVertical(float distance) {
  origin += upwards * distance;
}

void Camera::translateHorizontal(float distance) {
  origin += rightwards * distance;
}