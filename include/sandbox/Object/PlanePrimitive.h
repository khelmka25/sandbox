#pragma once

#include <glad/glad.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "Object/Object.h"
#include "Graphics/Shader.h"

namespace sb {
class PlanePrimitive : public Object {
 public:
  PlanePrimitive() : Object({}) {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // allocate enough space for everything
    auto const size = sizeof(vertices) + sizeof(normals) + sizeof(uvs);
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
    // buffer each vector in its own space
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(normals), normals);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(normals), sizeof(uvs), uvs);

    // vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)(sizeof(vertices)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)(sizeof(vertices) + sizeof(normals)));

    // element buffer
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), (void*)indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
  }

  virtual void draw(Shader* shader) {
    shader->setMat4("model", modelMatrix);
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
  }

 protected:
  static glm::vec3 vertices[4];
  static glm::vec3 normals[4];
  static glm::vec2 uvs[4];
  static unsigned indices[6];

 protected:
  // vbo, vao, ebo
  unsigned vbo, vao, ebo;
};
}  // namespace sb