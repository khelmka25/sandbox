#include "Camera/ScrollCamera.h"

#include <GLFW/glfw3.h>

#include <glm/gtc/quaternion.hpp>

#include "Camera/Camera.h"
#include "Data.h"

namespace sb {
void ScrollCamera::processEvent(const KeyboardEvent& e) noexcept {
  return;
}

void ScrollCamera::processEvent(const MouseButtonEvent& e) noexcept {
  switch (e.button) {
    case GLFW_MOUSE_BUTTON_LEFT: {
      if (std::cmp_equal(e.action, GLFW_PRESS)) {
        mouseLeftPressed = true;
      } else if (std::cmp_equal(e.action, GLFW_RELEASE)) {
        mouseLeftPressed = false;
      }
      break;
    }
    case GLFW_MOUSE_BUTTON_MIDDLE: {
      if (std::cmp_equal(e.action, GLFW_PRESS)) {
        mouseMiddlePressed = true;
      } else if (std::cmp_equal(e.action, GLFW_RELEASE)) {
        mouseMiddlePressed = false;
      }
      break;
    }
  }
}

void ScrollCamera::processEvent(const MousePositionEvent& e) noexcept {
  const glm::vec2 delta(e.xpos - prevCursorPos.x, e.ypos - prevCursorPos.y);
  
  // save previous state
  prevCursorPos.x = e.xpos;
  prevCursorPos.y = e.ypos;

  // if middle mouse pressed, rotate about origin
  // if ctrl + middle mouse pressed, pan about viewport
  if (mouseLeftPressed) {
    // rotate about origin, pretty involved process...
    translate(delta.x * positionSensitivity, delta.y * positionSensitivity);
    recompute();
  }
}

void ScrollCamera::processEvent(const ScrollEvent& e) noexcept {
  zoom(e.dy * scrollSensitivity);
  recompute();
}

void ScrollCamera::recompute() {
  // Calculate the position vector
  forwards = glm::vec3(0.f, 0.f, 1.f);
  // Calculate the rightwards vector
  const auto worldUp = glm::vec3(0, 1, 0);
  rightwards = glm::normalize(glm::cross(forwards, worldUp));
  // Calculate the upwards vector
  upwards = glm::normalize(glm::cross(rightwards, forwards));

  // Calculate the view matrix
  viewMatrix = glm::lookAt(positionVector + origin, origin, upwards);

  // Calculate the projection matrix
  projMatrix = glm::ortho(vpLeft / radius, vpRight / radius, vpBottom / radius,
                          vpTop / radius, -0xfff.p0f, 0xfff.p0f);
}

void ScrollCamera::zoom(float distance) {
  radius = std::max(minRadius, radius - distance);
}

void ScrollCamera::translate(float dx, float dy) {
  origin += rightwards * dx;
  origin += upwards * dy;
}
}  // namespace sb