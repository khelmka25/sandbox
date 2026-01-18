#pragma once

#include <glm/glm.hpp>

#include "Graphics/Texture/UvRect.h"

struct TextureDescriptor {
 public:
  TextureDescriptor() = default;
  TextureDescriptor(float t_width, float t_height)
      : width(t_width), height(t_height) {}

  // 2 position creation
  UvRect createSprite(glm::vec2 bl, glm::vec2 tr) const {
    UvRect output{};
    // convert to unit coords
    output.origin.x = bl.x / width;
    output.origin.y = bl.y / height;
    output.dims.x = (tr.x - bl.x) / width;
    output.dims.y = (tr.y - bl.y) / height;
    return output;
  }

  // position and size creation
  UvRect createSpritePs(glm::vec2 bl, glm::vec2 size) const {
    UvRect output{};
    // convert to unit coords
    output.origin.x = bl.x / width;
    output.origin.y = bl.y / height;
    output.dims.x = size.x / width;
    output.dims.y = size.y / height;
    return output;
  }

  // texture dimensions in pixels
  float width, height;
};