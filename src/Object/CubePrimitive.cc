#include "Object/CubePrimitive.h"

#include <glad/glad.h>

#include "Graphics/Shader.h"

namespace sb {

// ccw winding order:
//          ^
//      3-- | --2
//      |T1 | / |
// <------- / ------>
//      | / | T0|
//      0-- | --1
//          V
// T0: 0, 1, 2
// T1: 0, 2, 3
glm::vec3 CubePrimitive::vertices[24] = {
    // face +x
    {+1.0f, -1.0f, -1.0f}, // 2
    {+1.0f, +1.0f, -1.0f}, // 1
    {+1.0f, +1.0f, +1.0f}, // 0
    {+1.0f, -1.0f, +1.0f}, // 3
    // face +y
    {+1.0f, +1.0f, +1.0f},
    {+1.0f, +1.0f, -1.0f},
    {-1.0f, +1.0f, -1.0f},
    {-1.0f, +1.0f, +1.0f},
    // face +z
    {-1.0f, -1.0f, +1.0f},
    {+1.0f, -1.0f, +1.0f},
    {+1.0f, +1.0f, +1.0f},
    {-1.0f, +1.0f, +1.0f},

    // face -x
    {-1.0f, +1.0f, +1.0f},
    {-1.0f, +1.0f, -1.0f},
    {-1.0f, -1.0f, -1.0f},
    {-1.0f, -1.0f, +1.0f},
    // face -y
    {+1.0f, -1.0f, +1.0f},
    {-1.0f, -1.0f, +1.0f},
    {-1.0f, -1.0f, -1.0f},
    {+1.0f, -1.0f, -1.0f},
    // face -z
    {+1.0f, +1.0f, -1.0f},
    {+1.0f, -1.0f, -1.0f},
    {-1.0f, -1.0f, -1.0f},
    {-1.0f, +1.0f, -1.0f},
};

glm::vec3 CubePrimitive::normals[24] = {
    // face +x
    {+1.0f, +0.0f, +0.0f},
    {+1.0f, +0.0f, +0.0f},  
    {+1.0f, +0.0f, +0.0f},
    {+1.0f, +0.0f, +0.0f},
    // face +y
    {+0.0f, +1.0f, +0.0f},
    {+0.0f, +1.0f, +0.0f},
    {+0.0f, +1.0f, +0.0f},
    {+0.0f, +1.0f, +0.0f},
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
    // face -y
    {+0.0f, -1.0f, +0.0f},
    {+0.0f, -1.0f, +0.0f},
    {+0.0f, -1.0f, +0.0f},
    {+0.0f, -1.0f, +0.0f},
    // face -z
    {+0.0f, +0.0f, -1.0f},
    {+0.0f, +0.0f, -1.0f},
    {+0.0f, +0.0f, -1.0f},
    {+0.0f, +0.0f, -1.0f},
};  

glm::vec2 CubePrimitive::uvs[24] = {
    // face +x
    {0.0f, 0.0f},  
    {1.0f, 0.0f},  
    {1.0f, 1.0f},  
    {0.0f, 1.0f},  
    // face +y
    {0.0f, 0.0f},  
    {1.0f, 0.0f},  
    {1.0f, 1.0f},  
    {0.0f, 1.0f},  
    // face +z
    {0.0f, 0.0f},  
    {1.0f, 0.0f},  
    {1.0f, 1.0f},  
    {0.0f, 1.0f},  
    // face -x
    {0.0f, 0.0f},  
    {1.0f, 0.0f},  
    {1.0f, 1.0f},  
    {0.0f, 1.0f},  
    // face -y
    {0.0f, 0.0f},  
    {1.0f, 0.0f},  
    {1.0f, 1.0f},  
    {0.0f, 1.0f},  
    // face -z
    {0.0f, 0.0f},  
    {1.0f, 0.0f},  
    {1.0f, 1.0f},  
    {0.0f, 1.0f},  
};
  
// ccw winding order:
//          ^
//      3-- | --2
//      |T1 | / |
// <------- / ------>
//      | / | T0|
//      0-- | --1
//          V
// T0: 0, 1, 2
// T1: 0, 2, 3
// o+0, o+1, o+2, o+0, o+2, o+3
unsigned CubePrimitive::indices[36] = {
    // face +x (+0)
    0, 1, 2, 0, 2, 3,
    // face +y (+4)
    4, 5, 6, 4, 6, 7,
    // face +z (+8)
    8, 9, 10, 8, 10, 11,
    // face -x (+12)
    12, 13, 14, 12, 14, 15,
    // face -y (+16)
    16, 17, 18, 16, 18, 19,
    // face -z (+20)
    20, 21, 22, 20, 22, 23,
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
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBindVertexArray(0);
}
}  // namespace sb