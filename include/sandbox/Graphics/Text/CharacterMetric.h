#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace sb {
struct CharacterMetric {
  // unit coordinates
  glm::vec2 uv_tr;
  glm::vec2 uv_bl;

  // save the following:
  float horiAdvance;
  float vertAdvance;

  float horiBearing;
  float vertBearing;

  // width of the glyph in pixels
  float width;
  float height;
};
}