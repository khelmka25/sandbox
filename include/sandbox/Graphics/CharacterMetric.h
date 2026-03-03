#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace sb {
struct CharacterMetric {
  // save the following:
  unsigned horiAdvance;
  unsigned vertAdvance;

  unsigned horiBearing;
  unsigned vertBearing;

  // width of the glyph in pixels
  unsigned width;
  unsigned height;
};
}