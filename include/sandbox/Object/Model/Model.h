#pragma once

#include <filesystem>
#include <vector>

namespace sb {
// a model can have one or more associated meshes
class Model : public Object {
 public:
  // constructor, expects a filepath to a 3D model.
  Model(std::filesystem::path path) noexcept(true);

  ~Model() noexcept(true) = default;

  void draw(class Shader* shader) noexcept(true);

 protected:
  // model data
  std::vector<std::unique_ptr<class Mesh>> meshes;

  void importModel(std::filesystem::path filepath) noexcept(true);

  // processes a node in a recursive fashion. Processes each individual mesh
  // located at the node and repeats this process on its children nodes (if
  // any).
  void processNode(class aiNode* node, const class aiScene* scene) noexcept(true);

  std::unique_ptr<Mesh> processMesh(class aiMesh* mesh, const class aiScene* scene) noexcept(false);
};
}  // namespace sb