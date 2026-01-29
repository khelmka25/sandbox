#pragma once

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Object/Object.h"

namespace sb {
struct Orbit : public Object {
  Orbit() : Object(glm::vec3(0)) {
    // just a point at the origin
    glm::vec3 orbit(0);
    glm::vec3 albedo(1, 1, 0);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(orbit) + sizeof(albedo), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(orbit), glm::value_ptr(orbit));
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(orbit), sizeof(albedo), glm::value_ptr(albedo));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)sizeof(glm::vec3));
    glBindVertexArray(0);
  }

  void draw(Shader* shader) override {
    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, 1);
    glBindVertexArray(0);
  }

  unsigned vao, vbo;
};
}  // namespace sb