#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <iostream>

#include "Object/Object.h"

// describes a graphical 3D axis
struct Axes : public Object {
  Axes(glm::vec3 t_center) : Object(t_center) {
    // each axis is 3 lines:
    // ------ -- ------
    // ^              ^
    // |---length-----|
    // each long segment is 6/16 * L
    // each short segment is 2/16 * L
    // spacing is 1/16 * L

    // 3 lines
    auto origin = glm::vec3(0, 0, 0);
    auto length = 1.f;

    // directions
    auto x = glm::vec3(length, 0, 0);
    auto y = glm::vec3(0, length, 0);
    auto z = glm::vec3(0, 0, length);

    float large = 6.f / 16.f;
    float small = 2.f / 16.f;
    float spacing = 1.f / 16.f;

    glm::vec3 vertexData[] = {
        // x line
        origin,
        origin + (large) * (x),
        origin + (large + spacing) * x,
        origin + (large + spacing + small) * x,
        origin + (large + spacing + small + spacing) * x,
        origin + x,
        // y line
        origin,
        origin + (large) * (y),
        origin + (large + spacing) * y,
        origin + (large + spacing + small) * y,
        origin + (large + spacing + small + spacing) * y,
        origin + y,
        // z line
        origin,
        origin + (large) * (z),
        origin + (large + spacing) * z,
        origin + (large + spacing + small) * z,
        origin + (large + spacing + small + spacing) * z,
        origin + z,
    };

    // colors
    auto r = glm::vec3(1, 0, 0);
    auto g = glm::vec3(0, 1, 0);
    auto b = glm::vec3(0, 0, 1);

    glm::vec3 colorData[] = {
        // x line
        r,
        r,
        r,
        r,
        r,
        r,
        // y line
        g,
        g,
        g,
        g,
        g,
        g,
        // z line
        b,
        b,
        b,
        b,
        b,
        b,
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData) + sizeof(colorData), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexData), &vertexData);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertexData), sizeof(colorData), &colorData);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)sizeof(vertexData));

    glBindVertexArray(0);
  }

  void draw(Shader* shader) override {
    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, 18);
    glBindVertexArray(0);
  }

  unsigned vbo;
  unsigned vao;
};