
// #pragma once

// #include <glad/glad.h>  // holds all OpenGL type declarations

// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// #include <string>
// #include <vector>

// #include "Graphics/Shader.h"
// #include "Model/Vertex.h"

// struct Texture {
//   unsigned int id;
//   string type;
//   string path;
// };

// class Mesh {
//  public:
//   // constructor
//   Mesh(std::vector<Vertex>& t_vertices, std::vector<unsigned int>& t_indices, std::vector<Texture>& t_textures);
//   Mesh(std::vector<Vertex>&& t_vertices, std::vector<unsigned int>&& t_indices, std::vector<Texture>&& t_textures);

//   // render the mesh
//   void draw(Shader* shader) const;

//   ~Mesh() noexcept(true);

//  protected:
//   friend class Model;
//   // mesh Data
//   std::vector<Vertex> vertices;
//   std::vector<unsigned int> indices;
//   std::vector<Texture> textures;
  
//   unsigned int VAO;
//   // render data
//   unsigned int VBO, EBO;
// };
