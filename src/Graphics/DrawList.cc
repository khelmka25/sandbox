#include "Graphics/DrawList.h"

#include <glad/glad.h>

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
  beginPrimitive(GL_TRIANGLES);
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
}  // namespace sb