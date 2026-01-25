
#pragma once

#include <glad/glad.h>  // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <utility>
#include <vector>

#include "Graphics/Shader.h"
#include "Object/Model/Texture.h"
#include "Object/Model/Vertex.h"

class Mesh {
 public:
  Mesh() = default;
  explicit Mesh(std::vector<Vertex>&& t_vertices, std::vector<unsigned int>&& t_indices,
                std::vector<Texture>&& t_textures)
      : vertices(std::move(t_vertices)), indices(std::move(t_indices)), textures(std::move(t_textures)) {}

  Mesh(Mesh&& other)
      : vertices(std::move(other.vertices)),
        indices(std::move(other.indices)),
        textures(std::move(other.textures)),
        VAO(other.VAO),
        VBO(other.VBO),
        EBO(other.EBO) {
    // nullify graphics objects
    other.VAO = 0;
    other.VBO = 0;
    other.EBO = 0;
  }

  Mesh& operator=(Mesh&& other) {
    vertices = std::move(other.vertices);
    indices = std::move(other.indices);
    textures = std::move(other.textures);
    // copy and nullify graphics objects
    VAO = other.VAO;
    VBO = other.VBO;
    EBO = other.EBO;
    other.VAO = 0;
    other.VBO = 0;
    other.EBO = 0;

    return *this;
  }

  Mesh(const Mesh& other) = delete;
  Mesh& operator=(const Mesh& other) = delete;

  ~Mesh() noexcept(true) {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
  }

  void draw(Shader* shader) const {
    // draw the mesh
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
  }

  void bufferData() {
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // initializes all the buffer objects/arrays
    glBindVertexArray(VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // A great thing about structs is that their memory layout is sequential for
    // all its items. The effect is that we can simply pass a pointer to the
    // struct and it translates perfectly to a glm::vec3/2 array which again
    // translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
  }

 protected:
  // mesh Data
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  unsigned int VAO;
  // render data
  unsigned int VBO, EBO;
};
