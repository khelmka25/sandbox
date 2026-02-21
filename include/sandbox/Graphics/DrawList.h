#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <vector>
#include <iostream>

#include <glad/glad.h>

namespace sb {
struct DrawList {
 public:
  using VertexType = glm::vec3;
  using ColorType = glm::vec4;
  using UvType = glm::vec2;
  using IndexType = unsigned;

  DrawList();
  void clear();
  void rebuffer();

 public:
  void addText(glm::vec2 position, std::string_view text, class Atlas* atlas, glm::vec4 color);

 protected:
  // element data (structure of arrays)
  std::vector<VertexType> vertices;
  std::vector<ColorType> colors;
  std::vector<UvType> uvs;
  // indices
  std::vector<IndexType> indices;

 protected:
  unsigned bufferElementCapacity;
  unsigned bufferIndexCapacity;

  unsigned vbo;
  unsigned vao;
  unsigned ebo;
};
} 