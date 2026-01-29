#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>

#include "Object/Object.h"

struct Grid : public Object {
  Grid() : Object(glm::vec3(0)) {
    int count = 10;
    float spacing = 1.f;

    std::vector<glm::vec3> vertices;

    // create horizontal lines
    float x0 = -1.f * float(count / 2);
    for (int i = 0; i <= count; i++) {
      float x = x0 + i * spacing;
      float z1 = -1.f * float(count / 2);
      float z2 = +1.f * float(count / 2);

      vertices.emplace_back(x, 0, z1);
      vertices.emplace_back(x, 0, z2);
    }

    // create vertical lines
    float z0 = -1.f * float(count / 2);
    for (int i = 0; i <= count; i++) {
      float z = z0 + i * spacing;
      float x1 = -1.f * float(count / 2);
      float x2 = +1.f * float(count / 2);

      vertices.emplace_back(x1, 0, z);
      vertices.emplace_back(x2, 0, z);
    }

    // upload the data
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

    glBindVertexArray(0);
  }

  void draw(Shader* shader) override {
    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, 121);
    glBindVertexArray(0);
  }

  unsigned vbo;
  unsigned vao;
};