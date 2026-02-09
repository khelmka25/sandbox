#include "Object/CubePrimitive.h"

#include <glad/glad.h>

#include "Graphics/Shader.h"

namespace sb {

glm::vec3 CubePrimitive::vertices[24] = {
    // face -z
    {-1.0f, -1.0f, -1.0f},
    {+1.0f, -1.0f, -1.0f},
    {+1.0f, +1.0f, -1.0f},
    {-1.0f, +1.0f, -1.0f},
    // face +z
    {-1.0f, -1.0f, +1.0f},  // bl
    {+1.0f, -1.0f, +1.0f},  // br
    {+1.0f, +1.0f, +1.0f},  // tr
    {-1.0f, +1.0f, +1.0f},  // tl
    // face -x
    {-1.0f, +1.0f, +1.0f},
    {-1.0f, +1.0f, -1.0f},
    {-1.0f, -1.0f, -1.0f},
    {-1.0f, -1.0f, +1.0f},
    // face +x
    {+1.0f, +1.0f, +1.0f},
    {+1.0f, +1.0f, -1.0f},
    {+1.0f, -1.0f, -1.0f},
    {+1.0f, -1.0f, +1.0f},
    // face -y
    {-1.0f, -1.0f, -1.0f},
    {+1.0f, -1.0f, -1.0f},
    {+1.0f, -1.0f, +1.0f},
    {-1.0f, -1.0f, +1.0f},
    // face +y
    {-1.0f, +1.0f, -1.0f},
    {+1.0f, +1.0f, -1.0f},
    {+1.0f, +1.0f, +1.0f},
    {-1.0f, +1.0f, +1.0f},
};

glm::vec3 CubePrimitive::normals[24] = {
    // face -z
    {+0.0f, +0.0f, -1.0f},
    {+0.0f, +0.0f, -1.0f},
    {+0.0f, +0.0f, -1.0f},
    {+0.0f, +0.0f, -1.0f},

    // face +z
    {+0.0f, +0.0f, +1.0f},
    {+0.0f, +0.0f, +1.0f},
    {+0.0f, +0.0f, +1.0f},
    {+0.0f, +0.0f, +1.0f},

    // face -x
    {-1.0f, +0.0f, +0.0f},
    {-1.0f, +0.0f, +0.0f},
    {-1.0f, +0.0f, +0.0f},
    {-1.0f, +0.0f, +0.0f},

    // face +x
    {+1.0f, +0.0f, +0.0f},
    {+1.0f, +0.0f, +0.0f},
    {+1.0f, +0.0f, +0.0f},
    {+1.0f, +0.0f, +0.0f},

    // face -y
    {+0.0f, -1.0f, +0.0f},
    {+0.0f, -1.0f, +0.0f},
    {+0.0f, -1.0f, +0.0f},
    {+0.0f, -1.0f, +0.0f},

    // face +y
    {+0.0f, +1.0f, +0.0f},
    {+0.0f, +1.0f, +0.0f},
    {+0.0f, +1.0f, +0.0f},
    {+0.0f, +1.0f, +0.0f},
};

glm::vec2 CubePrimitive::uvs[24] = {
    // face -z
    {0.0f, 0.0f},  
    {1.0f, 0.0f},  
    {1.0f, 1.0f},  
    {0.0f, 1.0f},  
    // face +z
    {0.0f, 0.0f},  // bl
    {1.0f, 0.0f},  // br
    {1.0f, 1.0f},  // tr
    {0.0f, 1.0f},  // tl
    // face -x
    {0.0f, 0.0f},  
    {1.0f, 0.0f},  
    {1.0f, 1.0f},  
    {0.0f, 1.0f},  
    // face +x
    {0.0f, 0.0f},  
    {1.0f, 0.0f},  
    {1.0f, 1.0f},  
    {0.0f, 1.0f},  
    // face -y
    {0.0f, 0.0f},  
    {1.0f, 0.0f},  
    {1.0f, 1.0f},  
    {0.0f, 1.0f},  
    // face +y
    {0.0f, 0.0f},  
    {1.0f, 0.0f},  
    {1.0f, 1.0f},  
    {0.0f, 1.0f},  
};

// tr---tl
// |t0/t1|
// br---bl
// tri0: tr, tl, br
// tri1: tl, bl, br
unsigned CubePrimitive::indices[36] = {
    // face -z (+0)
    0, 1, 2, 2, 3, 0,
    // face +z (+4)
    4, 5, 6, 6, 7, 4,
    // face -x (+8)
    8, 9, 10, 10, 11, 8,
    // face +x (+12)
    12, 13, 14, 14, 15, 12,
    // face -y (+16)
    16, 17, 18, 18, 19, 16,
    // face +y (+20)
    20, 21, 22, 22, 23, 20,
};

CubePrimitive::CubePrimitive(glm::vec3 t_center, glm::vec4 t_albedo) noexcept(true)
    : Object(t_center), albedo(t_albedo) {

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  // create empty buffer for all the data
  auto const size = sizeof(vertices) + sizeof(normals) + sizeof(uvs);
  glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
  
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(normals), normals);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(normals), sizeof(uvs), uvs);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)(sizeof(vertices)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)(sizeof(vertices) + sizeof(normals)));
  
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), (void*)indices, GL_STATIC_DRAW);

  glBindVertexArray(0);
}

CubePrimitive::~CubePrimitive() noexcept(true) {
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);
}

void CubePrimitive::draw(Shader* shader) {
  shader->setVec3("albedo", albedo);
  shader->setMat4("model", modelMatrix);
  shader->setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(modelMatrix))));
  // draw the vertices
  glBindVertexArray(vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
}
}  // namespace sb