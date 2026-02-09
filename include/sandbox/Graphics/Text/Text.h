#pragma once

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/vec2.hpp>
#include <tuple>
#include <vector>

#include "Graphics/Text/Atlas.h"

namespace sb {
struct Text {
  unsigned vao;
  unsigned vbo;

  unsigned triCount;
};

void createText(const char* str, float xpos, float ypos, const Atlas* atlas, Text* text) {
  // generate the vertex data:
  std::vector<glm::vec2> pos;
  std::vector<glm::vec2> uv;

  char* ptr = const_cast<char*>(str);

  while (*ptr != 0) {
    int c = *ptr;
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
    // triangle 1
    pos.push_back(tr);
    pos.push_back(tl);
    pos.push_back(br);
    // triangle 2
    pos.push_back(tl);
    pos.push_back(bl);
    pos.push_back(br);

    // clockwise winding order
    // tr---tl
    // |t0/t1|
    // br---bl
    const glm::vec2 uv_tr = glm::vec2(metric.uv_tr.x, metric.uv_tr.y);
    const glm::vec2 uv_tl = glm::vec2(metric.uv_bl.x, metric.uv_tr.y);
    const glm::vec2 uv_br = glm::vec2(metric.uv_tr.x, metric.uv_bl.y);
    const glm::vec2 uv_bl = glm::vec2(metric.uv_bl.x, metric.uv_bl.y);
    // uv 1
    uv.push_back(uv_tr);
    uv.push_back(uv_tl);
    uv.push_back(uv_br);
    // uv 2
    uv.push_back(uv_tl);
    uv.push_back(uv_bl);
    uv.push_back(uv_br);

    xpos += metric.horiAdvance;

    ptr++;
  }

  glGenVertexArrays(1, &text->vao);
  glBindVertexArray(text->vao);

  glGenBuffers(1, &text->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, text->vbo);
  glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec2) + uv.size() * sizeof(glm::vec2), nullptr,
               GL_DYNAMIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, pos.size() * sizeof(glm::vec2), glm::value_ptr(pos.front()));
  glBufferSubData(GL_ARRAY_BUFFER, (pos.size() * sizeof(glm::vec2)), uv.size() * sizeof(glm::vec2),
                  glm::value_ptr(uv.front()));

  // vec2 position, vec2 uv
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)(0));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)(pos.size() * sizeof(glm::vec2)));

  text->triCount = pos.size() / 3;

  glBindVertexArray(0);
}
}  // namespace sb