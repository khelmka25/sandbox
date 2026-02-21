#include "Graphics/DrawList.h"

#include <glad/glad.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Graphics/Text/Atlas.h"

namespace sb {
DrawList::DrawList() : bufferElementCapacity(1024), bufferIndexCapacity(1536) {
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  // allocate enough for 4096 elements at first
  glGenBuffers(1, &vao);
  glBindBuffer(GL_ARRAY_BUFFER, vao);
  unsigned const elementSize = sizeof(VertexType) + sizeof(ColorType) + sizeof(UvType);
  glBufferData(GL_ARRAY_BUFFER, bufferElementCapacity * elementSize, nullptr, GL_DYNAMIC_DRAW);
  // allocate enough for 4096 indices
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferElementCapacity * 3u, nullptr, GL_DYNAMIC_DRAW);

  // vertex attributes
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexType), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ColorType), (void*)(bufferElementCapacity * (sizeof(VertexType))));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(UvType), (void*)(bufferElementCapacity * (sizeof(VertexType) + sizeof(ColorType))));

  glBindVertexArray(0);
}

void DrawList::clear() {
  vertices.clear();
  colors.clear();
  uvs.clear();

  indices.clear();
}

void DrawList::rebuffer() {
  const auto oldCount = bufferElementCapacity;
  bufferElementCapacity = vertices.size();

  const bool doRebufferData = true;
  const bool doRebufferIndices = true;
  const bool doResize = (bufferElementCapacity > oldCount) ? true : false;
  const bool doResetAttributes = doResize;

  glBindVertexArray(vao);

  if (doRebufferData) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    if (doResize) {
      unsigned const elementSize = sizeof(VertexType) + sizeof(ColorType) + sizeof(UvType);
      glBufferData(GL_ARRAY_BUFFER, bufferElementCapacity * elementSize, nullptr, GL_DYNAMIC_DRAW);
    }
    // buffer the vertices
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(VertexType), vertices.data());
    // buffer the colors
    glBufferSubData(GL_ARRAY_BUFFER, bufferElementCapacity * sizeof(VertexType), colors.size() * sizeof(ColorType), colors.data());
    // buffer the uvs
    glBufferSubData(GL_ARRAY_BUFFER, bufferElementCapacity * (sizeof(VertexType) + sizeof(ColorType)), uvs.size() * sizeof(UvType), uvs.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  if (doRebufferIndices) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    if (doResize) {
      glBufferData(GL_ARRAY_BUFFER, bufferIndexCapacity * sizeof(IndexType), nullptr, GL_DYNAMIC_DRAW);
    }
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(IndexType), indices.data());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  // reset attributes
  if (doResetAttributes) {
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexType), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ColorType), (void*)(bufferElementCapacity * (sizeof(VertexType))));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(UvType), (void*)(bufferElementCapacity * (sizeof(VertexType) + sizeof(ColorType))));
  }

  glBindVertexArray(0);
}

void DrawList::addText(glm::vec2 position, std::string_view text, class Atlas* atlas, glm::vec4 color) {
  float xpos = position.x;
  float ypos = position.y;
  for (const int& c : text) {
    // add the character to the vertex array
    auto metric = atlas->metrics.at(c);
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
    const glm::vec2 tr = origin + glm::vec2(metric.horiBearing, metric.vertBearing);
    const glm::vec2 tl = origin + glm::vec2(metric.horiBearing + metric.width, metric.vertBearing);
    const glm::vec2 br = origin + glm::vec2(metric.horiBearing, metric.vertBearing - metric.height);
    const glm::vec2 bl = origin + glm::vec2(metric.horiBearing + metric.width, metric.vertBearing - metric.height);
    // vertices
    vertices.emplace_back(tr.x, tr.y, 0.f);
    vertices.emplace_back(tl.x, tl.y, 0.f);
    vertices.emplace_back(bl.x, bl.y, 0.f);
    vertices.emplace_back(br.x, br.y, 0.f);

    // colors: uniform
    colors.push_back(color);
    colors.push_back(color);
    colors.push_back(color);
    colors.push_back(color);

    // clockwise winding order
    // tr---tl
    // |t0/t1|
    // br---bl
    const glm::vec2 uv_tr = glm::vec2(metric.uv_tr.x, metric.uv_tr.y);
    const glm::vec2 uv_tl = glm::vec2(metric.uv_bl.x, metric.uv_tr.y);
    const glm::vec2 uv_br = glm::vec2(metric.uv_tr.x, metric.uv_bl.y);
    const glm::vec2 uv_bl = glm::vec2(metric.uv_bl.x, metric.uv_bl.y);
    // uvs
    uvs.emplace_back(uv_tr.x, uv_tr.y);
    uvs.emplace_back(uv_tl.x, uv_tl.y);
    uvs.emplace_back(uv_bl.x, uv_bl.y);
    uvs.emplace_back(uv_br.x, uv_br.y);

    xpos += metric.horiAdvance;

    // indices
    unsigned baseIndex = vertices.size();
    // triangle 1
    indices.push_back(baseIndex + 0);
    indices.push_back(baseIndex + 1);
    indices.push_back(baseIndex + 3);
    // triangle 2
    indices.push_back(baseIndex + 1);
    indices.push_back(baseIndex + 2);
    indices.push_back(baseIndex + 3);
  }
}
} 