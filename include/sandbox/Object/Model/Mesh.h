
#pragma once

#include <glad/glad.h>  // holds all OpenGL type declarations

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <string>
#include <utility>
#include <vector>

#include "Graphics/Shader.h"
#include "Object/Model/Material.h"
#include "Object/Model/Texture.h"
#include "Object/Model/Vertex.h"

namespace sb {
class Mesh {
 public:
  explicit Mesh(std::vector<Vertex>&& t_vertices, std::vector<unsigned int>&& t_indices, Material&& t_material);
  Mesh(Mesh&& other);

  Mesh& operator=(Mesh&& other);
  Mesh(const Mesh& other) = delete;
  Mesh& operator=(const Mesh& other) = delete;

  ~Mesh() noexcept(true);

  void draw(Shader* shader) const;

  void bufferData();

 protected:
  // mesh Data
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  Material material;

  unsigned int VAO;
  // render data
  unsigned int VBO, EBO;
};
}  // namespace sb
