#include "Graphics/DrawList.h"

#include <glad/glad.h>

#include <cmath>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Graphics/CharacterMetric.h"
#include "Graphics/CharacterMetricSet.h"
#include "Graphics/GeometryList.h"
#include "Graphics/TextureAtlas.h"

namespace sb {
DrawList::DrawList() : GeometryList() {}

void DrawList::clear() {
  drawCommands.clear();
  GeometryList::clear();
}

void DrawList::draw() {
  // end the current draw command - if any
  endPrimitive();

  glBindVertexArray(vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  for (const auto& drawCommand : drawCommands) {
    const std::size_t offset = drawCommand.begin;
    const std::size_t count = drawCommand.end - drawCommand.end;
    glDrawElements(drawCommand.primitive, count, GL_UNSIGNED_BYTE, (void*)offset);
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void DrawList::addText(glm::vec2 position, std::string_view text, glm::vec4 color, std::shared_ptr<TextureAtlas> atlas,
                       std::shared_ptr<CharacterMetricSet> metricSet) {
  beginTriangles();
  // draw text as a set of textured rects
  float xpos = position.x;
  const float ypos = position.y;

  // iterate through characters
  for (const int c : text) {
    // add the character to the vertex array
    const CharacterMetric& metric = metricSet->metrics.at(c);

    // clockwise winding order
    // tr---tl
    // |t0/t1|
    // br---bl
    // font glyph origin
    glm::vec2 origin(xpos, ypos);
    // |   tr---tl
    // |   |   / |
    // |   |0 / 1|
    // O - | /   | -----
    // |   br---bl
    // |
    const glm::vec2 p1(xpos + metric.horiBearing, ypos + metric.vertBearing);
    const glm::vec2 p2(xpos + metric.horiBearing + metric.width, ypos + metric.vertBearing - metric.height);

    const auto [uv1, uv2] = atlas->getCharacterUv(0, c);

    // build a textured rect
    buildRect(p1, p2, uv1, uv2, color);
    // advance the cursor
    xpos += metric.horiAdvance;
  }
}

void DrawList::addEllipse(glm::vec2 p1, glm::vec2 p2, glm::vec4 color) {
  beginTriangles();
  // goal: construct an elliptical polygon:
  // https://en.wikipedia.org/wiki/Ellipse
  // (x, y) = (a cos(t), b sin(t)), t := [0, 2pi]
  // width:  2 a
  // height: 2 b
  const float a = (p2.x - p1.x) / 2.f;
  const float b = (p2.y - p1.y) / 2.f;

  constexpr float two_pi = 2.f * std::numbers::pi_v<float>;
  unsigned segmentCount = 32u;
  const float dt = two_pi / static_cast<float>(segmentCount);
  // create points:
  std::vector<glm::vec2> out_points;
  // add the center as the first point
  out_points.emplace_back((p2.x + p1.x) / 2.f, (p2.y + p1.y) / 2.f);
  for (unsigned i = 0; i < segmentCount; i++) {
    const float t = 0.f + dt * static_cast<float>(i);
    out_points.emplace_back(a * std::cos(t), b * std::sin(t));
  }
  // add the point after the center to close the loop
  out_points.push_back(out_points.at(1ull));

  // create a vector of uvs using the blank texture
  // const auto [uv1, uv2] = atlas->getTextureUvs(0);
  const glm::vec2 uv1(0.f);
  std::vector<glm::vec2> out_uvs(out_points.size());
  std::fill(out_uvs.begin(), out_uvs.end(), uv1);

  // build the polygon:
  buildPolygon(out_points, out_uvs, color);
}

void DrawList::addEllipseOutline(glm::vec2 p1, glm::vec2 p2, glm::vec4 color, unsigned width) {
  beginLines(width);
  // goal: construct an elliptical polygon:
  // https://en.wikipedia.org/wiki/Ellipse
  // (x, y) = (a cos(t), b sin(t)), t := [0, 2pi]
  // width:  2 a
  // height: 2 b
  const float a = (p2.x - p1.x) / 2.f;
  const float b = (p2.y - p1.y) / 2.f;

  constexpr float two_pi = 2.f * std::numbers::pi_v<float>;
  unsigned segmentCount = 32u;
  const float dt = two_pi / static_cast<float>(segmentCount);
  // create points:
  std::vector<glm::vec2> out_points;
  // add the center as the first point
  out_points.emplace_back((p2.x + p1.x) / 2.f, (p2.y + p1.y) / 2.f);
  for (unsigned i = 0; i < segmentCount; i++) {
    const float t = 0.f + dt * static_cast<float>(i);
    out_points.emplace_back(a * std::cos(t), b * std::sin(t));
  }
  // add the point after the center to close the loop
  out_points.push_back(out_points.at(1ull));

  // create a vector of uvs using the blank texture
  // const auto [uv1, uv2] = atlas->getTextureUvs(0);
  const glm::vec2 uv1(0.f);
  std::vector<glm::vec2> out_uvs(out_points.size());
  std::fill(out_uvs.begin(), out_uvs.end(), uv1);

  // build the lines here
}

void DrawList::addLineSegment(glm::vec2 p1, glm::vec2 p2, glm::vec4 color, unsigned width) {
  beginLines(width);
  glm::vec2 uv1(0.f);
  glm::vec2 uv2(0.f);
  buildLine(p1, p2, uv1, uv2, color);
}

void DrawList::addLineSegments(std::vector<glm::vec2>& points, glm::vec4 color, unsigned width) {
  if (points.size() < 2ull) {
    return;
  }

  beginLines(width);

  for (std::size_t i = 0; i < points.size() - 1ull; i++) {
    const auto& p1 = points.at(i + 0ull);
    const auto& p2 = points.at(i + 1ull);
    glm::vec2 uv1(0.f);
    glm::vec2 uv2(0.f);
    buildLine(p1, p2, uv1, uv2, color);
  }
}

void DrawList::addPoint(glm::vec2 center, glm::vec4 color, unsigned size) {
  beginPoints(size);
  const glm::vec2 uv(0.f);
  buildPoint(center, uv, color);
}

void DrawList::addPolygon(std::vector<glm::vec2>& points, glm::vec4 color) {
  if (points.size() < 3ull) {
    return;
  }

  beginTriangles();
}

void DrawList::addPolygonOutline(std::vector<glm::vec2>& points, glm::vec4 color, unsigned width) {
  if (points.size() < 2ull) {
    return;
  }

  beginLines(width);

  for (std::size_t i = 0; i < points.size() - 1ull; i++) {
    const auto& p1 = points.at(i + 0ull);
    const auto& p2 = points.at(i + 1ull);
    glm::vec2 uv1(0.f);
    glm::vec2 uv2(0.f);
    buildLine(p1, p2, uv1, uv2, color);
  }
}

void DrawList::addTriangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec4 color) {
  beginTriangles();
  const glm::vec2 uv(0.f);
  buildTriangle(p1, p2, p3, uv, uv, uv, color);
}

void DrawList::addTriangleOutline(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec4 color, unsigned width) {
  beginLines(width);
  const glm::vec2 uv(0.f);
  buildLine(p1, p2, uv, uv, color);
  buildLine(p2, p3, uv, uv, color);
  buildLine(p3, p1, uv, uv, color);
}

void DrawList::addRect(glm::vec2 p1, glm::vec2 p2, glm::vec4 color) {
  beginTriangles();
  const glm::vec2 uv(0.f);
  buildRect(p1, p2, uv, uv, color);
}

void DrawList::addRectOutline(glm::vec2 p1, glm::vec2 p2, glm::vec4 color, unsigned width) {
  beginLines(width);
  const glm::vec2 uv(0.f);
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
  const glm::vec2 bl(p1.x, p2.y);
  const glm::vec2 br(p2.x, p2.y);
  const glm::vec2 tr(p2.x, p1.y);
  const glm::vec2 tl(p1.x, p1.y);
  // counterclockwise line drawing (non-important)
  buildLine(bl, br, uv, uv, color);
  buildLine(br, tr, uv, uv, color);
  buildLine(tr, tl, uv, uv, color);
  buildLine(tl, bl, uv, uv, color);
}

void DrawList::addCircle(glm::vec2 center, glm::vec4 color) {
  beginTriangles();
}

void DrawList::addCircleOutline(glm::vec2 center, glm::vec4 color, unsigned width) {
  beginLines(width);
}

void DrawList::addSector(glm::vec2 center, float t_start, float t_end, glm::vec4 color) {
  beginTriangles();
}

void DrawList::addSectorOutline(glm::vec2 center, float t_start, float t_end, glm::vec4 color, unsigned width) {
  beginLines(width);
}

void DrawList::addArc(glm::vec2 center, float t_start, float t_end, glm::vec4 color, unsigned width) {
  beginLines(width);
}

void DrawList::addTexturedRect(glm::vec2 p1, glm::vec2 p2, glm::vec4 color, std::shared_ptr<class TextureAtlas> atlas,
                               unsigned subTextureId) {
  beginTriangles();
}

void DrawList::beginTriangles() {
  if (!drawCommands.empty()) {
    auto& curr = drawCommands.back();
    if (curr.primitive == GL_TRIANGLES) {
      // do nothing, continue current command
      return;
    }
    // end current command
    curr.end = elementCount();
  }

  // create a new command
  DrawCommand next;
  next.begin = elementCount();
  next.primitive = GL_TRIANGLES;
  drawCommands.push_back(std::move(next));
}

void DrawList::beginLines(unsigned lineWidth) {
  if (!drawCommands.empty()) {
    auto& curr = drawCommands.back();
    if ((curr.primitive == GL_LINES) && (curr.lineWidth == lineWidth)) {
      // do nothing, continue current command
      return;
    }
    // end current command
    curr.end = elementCount();
  }

  // create a new command
  DrawCommand next;
  next.begin = elementCount();
  next.primitive = GL_LINES;
  next.lineWidth = lineWidth;
  drawCommands.push_back(std::move(next));
}

void DrawList::beginPoints(unsigned pointSize) {
  if (!drawCommands.empty()) {
    auto& curr = drawCommands.back();
    if ((curr.primitive == GL_POINTS) && (curr.pointSize == pointSize)) {
      // do nothing, continue current command
      return;
    }
    // end current command
    curr.end = elementCount();
  }

  // create a new command
  DrawCommand next;
  next.begin = elementCount();
  next.primitive = GL_POINTS;
  next.pointSize = pointSize;
  drawCommands.push_back(std::move(next));
}

void DrawList::endPrimitive() {
  if (!drawCommands.empty()) {
    auto& curr = drawCommands.back();
    // end current command
    curr.end = elementCount();
  }
}

}  // namespace sb