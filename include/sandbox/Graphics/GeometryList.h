#pragma once

#include <glad/glad.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <iostream>
#include <memory>
#include <vector>

namespace sb {
class GeometryList {
 public:
  GeometryList() = default;

 protected:
  bool empty();
  void clear();
  void rebuffer();

  std::size_t elementCount();

 protected:
  // https://pillow-wiredfool.readthedocs.io/en/latest/reference/ImageDraw.html
  // (0, 0) is designated as the top-left of the screen
  // like so:
  // o------> x
  // |
  // |
  // V
  // y

  // for a given pair of points, p1 & p2,
  // p1 will always be in the uppper left
  // and p2 will be the lower right

  // geometry, however, uses the following coordinate system:
  // ccw winding order:
  //          ^
  //      3-- | --2
  //      |T1 | / |
  // <------- / ------> +x
  //      | / | T0|
  //      0-- | --1
  //          V
  //         +y
  // T0: 0, 1, 2
  // T1: 0, 2, 3
  void buildPoint(glm::vec2 p, glm::vec2 uv, glm::vec4 color);
  void buildLine(glm::vec2 p1, glm::vec2 p2, glm::vec2 uv1, glm::vec2 uv2, glm::vec4 color);
  void buildTriangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3, glm::vec4 color);
  void buildRect(glm::vec2 p1, glm::vec2 p2, glm::vec2 uv1, glm::vec2 uv2, glm::vec4 color);
  void buildPolygon(std::vector<glm::vec2>& points, std::vector<glm::vec2>& uvs, glm::vec4 color);

 protected:
  unsigned vbo;
  unsigned vao;
  unsigned ebo;

 private:
  // element data (structure of arrays)
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec4> colors;
  std::vector<glm::vec2> uvs;
  // indices
  std::vector<unsigned int> indices;
};

}  // namespace sb