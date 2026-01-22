// #include "Model/Model.h"

// Model::Model(std::filesystem::path t_filepath) {
//   // path of the model
//   const auto pPath = t_filepath.string();
//   // flags for the importer
//   unsigned pFlags{};
//   pFlags |= aiProcess_Triangulate;
//   pFlags |= aiProcess_GenSmoothNormals;
//   pFlags |= aiProcess_FlipUVs;
//   pFlags |= aiProcess_CalcTangentSpace;
  
//   // attempt to import the file via ASSIMP
//   Assimp::Importer importer;
//   const aiScene* scene = importer.ReadFile(pPath, pFlags);

//   // check for errors
//   if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
//     cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
//     return;
//   }

//   // retrieve the directory path of the filepath
//   directory = path.substr(0, path.find_last_of('/'));
  
//   // process ASSIMP's root node recursively
//   processNode(scene->mRootNode, scene);
// }

// Model::~Model() noexcept(true) {}

// void Model::draw(Shader* shader) {
//   for (const auto& mesh : meshes) {
//     mesh.draw(shader);
//   }
// }