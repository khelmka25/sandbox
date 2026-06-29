#include "Camera/OrbitCamera.h"

#include <GLFW/glfw3.h>

#include <glm/gtc/quaternion.hpp>

#include "Camera/Camera.h"

namespace sb {
void OrbitCamera::processEvent(const KeyboardEvent& e) noexcept {
  switch (e.key) {
    case GLFW_KEY_LEFT_CONTROL: {
      if (std::cmp_equal(e.action, GLFW_PRESS)) {
        ctrlPressed = true;
      } else if (std::cmp_equal(e.action, GLFW_RELEASE)) {
        ctrlPressed = false;
      }
      break;
    }
    case GLFW_KEY_UP: {
      if (std::cmp_equal(e.action, GLFW_PRESS) || std::cmp_equal(e.action, GLFW_REPEAT)) {
        rotate(0.f, +1.f);
        recompute();
      }
      break;
    }
    case GLFW_KEY_DOWN: {
      if (std::cmp_equal(e.action, GLFW_PRESS) || std::cmp_equal(e.action, GLFW_REPEAT)) {
        rotate(0.f, -1.f);
        recompute();
      }
      break;
    }
    case GLFW_KEY_LEFT: {
      if (std::cmp_equal(e.action, GLFW_PRESS) || std::cmp_equal(e.action, GLFW_REPEAT)) {
        rotate(+1.f, 0.f);
        recompute();
      }
      break;
    }
    case GLFW_KEY_RIGHT: {
      if (std::cmp_equal(e.action, GLFW_PRESS) || std::cmp_equal(e.action, GLFW_REPEAT)) {
        rotate(-1.f, 0.f);
        recompute();
      }
    }
  }
}

void OrbitCamera::processEvent(const MouseButtonEvent& e) noexcept {
  switch (e.button) {
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

void OrbitCamera::processEvent(const MousePositionEvent& e) noexcept {
  auto dx = e.xpos - xposPrev;
  auto dy = e.ypos - yposPrev;

  xposPrev = e.xpos;
  yposPrev = e.ypos;

  // if middle mouse pressed, rotate about origin
  // if ctrl + middle mouse pressed, pan about viewport
  if (mouseMiddlePressed) {
    if (ctrlPressed) {
      // pan about viewport
      translate(dx * 0.01f, dy * 0.01f);
      recompute();
    } else {
      // // rotate about origin, pretty involved process...
      rotate(dx * 0.5f, dy * 0.5f);
      recompute();
    }
  }
}

void OrbitCamera::processEvent(const ScrollEvent& e) noexcept {
  zoom(e.dy * scrollSensitivity);
  recompute();
}

void OrbitCamera::recompute() {
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

  const float width = vpRight - vpLeft;
  const float height = vpTop - vpBottom;
  projMatrix = glm::perspective(glm::radians(70.f), width / height, 0.1f, 1000.0f);
}

void OrbitCamera::rotate(float dx, float dy) {
  theta = theta + dx;
  phi = std::clamp(phi + dy, -89.0f, 89.0f);
}

void OrbitCamera::zoom(float dr) {
  radius = std::max(minRadius, radius - dr);
}

void OrbitCamera::translate(float dx, float dy) {
  origin += rightwards * dx;
  origin += upwards * dy;
}
}  // namespace sb