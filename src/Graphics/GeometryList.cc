#include "Graphics/GeometryList.h"

namespace sb {
GeometryList::GeometryList() {
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);
}

bool GeometryList::empty() {
  return vertices.empty();
}

void GeometryList::clear() {
  vertices.clear();
  colors.clear();
  uvs.clear();

  indices.clear();
}

void GeometryList::rebuffer() {
  assert(vao != 0 && "Invalid VAO Name");
  assert(vbo != 0 && "Invalid VBO Name");
  assert(ebo != 0 && "Invalid EBO Name");

  glBindVertexArray(vao);

  // 1. Upload Vertex Data:
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  // base memory offset
  const std::size_t baseOffset = 0ull;
  // vertices offset and size
  const std::size_t verticesOffset = baseOffset + 0ull;
  const std::size_t verticesSize = vertices.size() * sizeof(decltype(vertices)::value_type);
  // colors offset and size
  const std::size_t colorsOffset = baseOffset + verticesSize;
  const std::size_t colorsSize = colors.size() * sizeof(decltype(colors)::value_type);
  // uvs offset and size
  const std::size_t uvsOffset = baseOffset + verticesSize + colorsSize;
  const std::size_t uvsSize = uvs.size() * sizeof(decltype(uvs)::value_type);

  // allocate enough space for everything
  const std::size_t bufferSize = verticesSize + colorsSize + uvsSize;
  glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_DYNAMIC_DRAW);
  // rebuffer the vertices:
  glBufferSubData(GL_ARRAY_BUFFER, verticesOffset, verticesSize, vertices.data());
  // rebuffer the colors:
  glBufferSubData(GL_ARRAY_BUFFER, colorsOffset, colorsSize, colors.data());
  // rebuffer the uvs:
  glBufferSubData(GL_ARRAY_BUFFER, uvsOffset, uvsSize, uvs.data());

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // 2. Upload Index Data:
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  // indices offset and size
  const std::size_t indicesOffset = 0ull;
  const std::size_t indicesSize = indices.size() * sizeof(decltype(indices)::value_type);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, nullptr, GL_DYNAMIC_DRAW);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indicesOffset, indicesSize, indices.data());

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // 3. Set up vertex attributes: struct of arrays
  // vertices
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(decltype(vertices)::value_type), (void*)verticesOffset);
  // colors
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(decltype(colors)::value_type), (void*)(colorsOffset));
  // uvs
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(decltype(uvs)::value_type), (void*)(uvsOffset));

  glBindVertexArray(0);
}

std::size_t GeometryList::elementCount() {
  return indices.size();
}

void GeometryList::buildPoint(glm::vec2 p, glm::vec2 uv, glm::vec4 color) {
  const std::size_t offset = vertices.size();
  vertices.emplace_back(p.x, p.y, 0.f);
  colors.push_back(color);
  uvs.push_back(uv);
  
  indices.push_back(offset);
}

void GeometryList::buildLine(glm::vec2 p1, glm::vec2 p2, glm::vec2 uv1, glm::vec2 uv2, glm::vec4 color) {
  const std::size_t offset = vertices.size();

  // vertices: given
  vertices.emplace_back(p1.x, p1.y, 0.f);
  vertices.emplace_back(p2.x, p2.y, 0.f);
  // colors: given
  colors.push_back(color);
  colors.push_back(color);
  // uvs: given
  uvs.push_back(uv1);
  uvs.push_back(uv2);

  // indices
  indices.push_back(offset + 0ull);
  indices.push_back(offset + 1ull);
}

void GeometryList::buildTriangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3,
                                 glm::vec4 color) {
  const std::size_t offset = vertices.size();
  
  // vertices: given
  vertices.emplace_back(p1.x, p1.y, 0.f);
  vertices.emplace_back(p2.x, p2.y, 0.f);
  vertices.emplace_back(p3.x, p3.y, 0.f);
  // colors: given
  colors.push_back(color);
  colors.push_back(color);
  colors.push_back(color);
  // uvs: given
  uvs.push_back(uv1);
  uvs.push_back(uv2);
  uvs.push_back(uv3);
  
  // indices
  indices.push_back(offset + 0ull);
  indices.push_back(offset + 1ull);
  indices.push_back(offset + 2ull);
}

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
//   p1=3-- | --2
//      |T1 | / |
// <------- / ------> +x
//      | / | T0|
//      0-- | --1=p2
//          V
//         +y
// T0: 0, 1, 2
// T1: 0, 2, 3
void GeometryList::buildRect(glm::vec2 p1, glm::vec2 p2, glm::vec2 uv1, glm::vec2 uv2, glm::vec4 color) {
  const std::size_t offset = vertices.size();

  // vertices: given
  vertices.emplace_back(p1.x, p2.y, 0.f);
  vertices.emplace_back(p2.x, p2.y, 0.f);
  vertices.emplace_back(p2.x, p1.y, 0.f);
  vertices.emplace_back(p1.x, p1.y, 0.f);
  // colors: given
  colors.push_back(color);
  colors.push_back(color);
  colors.push_back(color);
  colors.push_back(color);
  // uvs: given
  uvs.emplace_back(uv1.x, uv2.y);
  uvs.emplace_back(uv2.x, uv2.y);
  uvs.emplace_back(uv2.x, uv1.y);
  uvs.emplace_back(uv1.x, uv1.y);

  // indices: triangle 1
  indices.push_back(offset + 0ull);
  indices.push_back(offset + 1ull);
  indices.push_back(offset + 2ull);
  // indices: triangle 2
  indices.push_back(offset + 3ull);
  indices.push_back(offset + 2ull);
  indices.push_back(offset + 0ull);
}

void GeometryList::buildPolygon(std::vector<glm::vec2>& in_points, std::vector<glm::vec2>& in_uvs, glm::vec4 color) {
  const std::size_t offset = vertices.size();

  // the first point and uvs is required to be the center
  const auto& p1 = in_points.front();
  const auto& uv1 = in_uvs.front();
  vertices.emplace_back(p1.x, p1.y, 0.f);
  colors.push_back(color);
  uvs.push_back(uv1);

  // build the triangles in the polygon
  for (long long i = 1ll; i < (in_points.size() - 1ll); i++) {
    // build a triangle:
    const auto& p2 = in_points.at(i + 0ll);
    const auto& p3 = in_points.at(i + 1ll);
    const auto& uv2 = in_uvs.at(i + 0ll);
    const auto& uv3 = in_uvs.at(i + 1ll);
    // vertices 2, 3
    vertices.emplace_back(p2.x, p2.y, 0.f);
    vertices.emplace_back(p3.x, p3.y, 0.f);
    // colors 2, 3
    colors.push_back(color);
    colors.push_back(color);
    // uvs 2, 3
    uvs.push_back(uv2);
    uvs.push_back(uv3);
    // elements:
    indices.push_back(offset + 1);
    indices.push_back(offset + i * 2 + 0);
    indices.push_back(offset + i * 2 + 1);
  }
}

}  // namespace sb