#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glad/glad.h>
#include <stb_image.h>

#include <assimp/Importer.hpp>
#include <filesystem>
#include <fstream>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "Graphics/Shader.h"
#include "Graphics/Texture/Texture.h"
#include "Object/Model/Mesh.h"
#include "Object/Model/Texture.h"
#include "Object/Model/Vertex.h"
#include "Object/Object.h"

#include "Object/Model/Model.h"

namespace sb {
// constructor, expects a filepath to a 3D model.
Model::Model(std::filesystem::path path) noexcept(true) : Object(glm::vec3(0)) {
  importModel(path);

  for (auto& mesh : meshes) {
    mesh->bufferData();
  }
}

void Model::draw(Shader* shader) noexcept(true) {
  for (const auto& mesh : meshes) {
    mesh->draw(shader);
  }
}

void Model::importModel(std::filesystem::path filepath) noexcept(true) {
  /* Attempt to open the model */
  Assimp::Importer import;
  unsigned pFlags{};
  pFlags |= aiProcess_Triangulate;
  pFlags |= aiProcess_GenSmoothNormals;
  pFlags |= aiProcess_FlipUVs;
  pFlags |= aiProcess_CalcTangentSpace;
  pFlags |= aiProcess_JoinIdenticalVertices;
  const aiScene* scene = import.ReadFile(filepath, pFlags);
  // check that the model was imported correctly
  if ((scene == nullptr) || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || (scene->mRootNode == nullptr)) {
    std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
    return;
  }

  processNode(scene->mRootNode, scene);

  // print data about the scene
  std::cout << "Imported:  " << filepath << std::endl;
  std::cout << "Materials: " << scene->mNumMaterials << std::endl;
  std::cout << "Meshes:    " << scene->mNumMeshes << std::endl;
}

// processes a node in a recursive fashion. Processes each individual mesh
// located at the node and repeats this process on its children nodes (if
// any).
void Model::processNode(aiNode* node, const aiScene* scene) noexcept(true) {
  // process each mesh located at the current node
  for (unsigned i = 0; i < node->mNumMeshes; i++) {
    // the node object only contains indices to index the actual objects in
    // the scene. the scene contains all the data, node is just to keep stuff
    // organized (like relations between nodes).
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    try {
      auto output = processMesh(mesh, scene);
      meshes.push_back(std::move(output));
    } catch (const std::exception& e) {
      std::cout << e.what() << std::endl;
    }
  }

  // after we've processed all of the meshes (if any) we then recursively
  // process each of the children nodes
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene);
  }
}

std::unique_ptr<Mesh> Model::processMesh(aiMesh* mesh, const aiScene* scene) noexcept(false) {
  // Check for vertex positions
  if (!mesh->HasPositions()) {
    std::cout << "Mesh has no vertex positions!" << std::endl;
    throw std::runtime_error("Invalid Mesh");
  }

  // Check for vertex normals
  if (!mesh->HasNormals()) {
    std::cout << "Mesh has no vertex normals!" << std::endl;
    throw std::runtime_error("Invalid Mesh");
  }

  // check for faces
  if (!mesh->HasFaces()) {
    std::cout << "Mesh contains no faces!" << std::endl;
    throw std::runtime_error("Invalid Mesh");
  }

  // output data: vertex
  std::vector<Vertex> vertices;

  // walk through each of the mesh's vertices
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex;

    // positions
    vertex.Position.x = mesh->mVertices[i].x;
    vertex.Position.y = mesh->mVertices[i].y;
    vertex.Position.z = mesh->mVertices[i].z;

    // normals
    if (mesh->HasNormals()) {
      vertex.Normal.x = mesh->mNormals[i].x;
      vertex.Normal.y = mesh->mNormals[i].y;
      vertex.Normal.z = mesh->mNormals[i].z;
    }

    if (mesh->HasTextureCoords(i)) {
      // a vertex can contain up to 8 different texture coordinates. We thus
      // make the assumption that we won't use models where a vertex can have
      // multiple texture coordinates so we always take the first set (0).
      vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
      vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
    } else {
      vertex.TexCoords.x = 0.f;
      vertex.TexCoords.y = 0.f;
    }

    vertices.push_back(vertex);
  }

  // output: indices
  std::vector<unsigned int> indices;

  // now wak through each of the mesh's faces (a face is a mesh its triangle)
  // and retrieve the corresponding vertex indices.
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    const aiFace& face = mesh->mFaces[i];
    // retrieve all indices of the face and store them in the indices vector
    for (unsigned int j = 0; j < face.mNumIndices; j++) {
      indices.push_back(face.mIndices[j]);
    }
  }

  // retrieve material: may be default or otherwise
  if (mesh->mMaterialIndex > 0) {
    aiColor4D diffuse;
    aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
    if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &diffuse)) {
      Material material;
      material.albedo = glm::vec3(diffuse.r, diffuse.g, diffuse.b);
      // return a mesh object created from the extracted mesh data
      return std::make_unique<Mesh>(std::move(vertices), std::move(indices), std::move(material));
    }
  }

  return std::make_unique<Mesh>(std::move(vertices), std::move(indices), Material{});
}
}  // namespace sb