#pragma once

#include <vector>
#include <filesystem>
#include <utility>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace sb {
// goal: provide a way to combine separate textures, font atlases, and others
// into a single texture for use in user interfaces
// the main texture produced and uploaded to the gpu has many
// subtextures that can be accessed via uv coordinates
// these uv coordinates are available via named lookup in the TextureAtlas
struct TextureAtlas {
  // this texture is used for widgets, it contains a fully white, opaque texture
  unsigned addBlankTexture();
  // this is used to signify that a texture index name is invalid
  unsigned addMissingTexture();

  // add a single texture, and return the referencing name
  [[nodiscard]]
  unsigned addTexture(std::filesystem::path filepath) noexcept(false);

  [[nodiscard]]
  std::pair<glm::vec2, glm::vec2> getTextureUv(unsigned textureName) noexcept(false);

  // add a font (multiple textures) and return the referencing name
  [[nodiscard]]
  unsigned addFont(std::filesystem::path filepath, int size) noexcept(false);

  [[nodiscard]]
  std::pair<glm::vec2, glm::vec2> getCharacterUv(unsigned fontBaseName, int c) noexcept(false);

  // recompute the locations of the subtextures
  void recompute();

  // upload the finished texture to the gpu
  // based on the recomputed locations, create a texture on the gpu
  // and upload all subtextures into this texture
  void rebuffer();

  struct TextureDesciptor {
    glm::ivec2 position;
    glm::ivec2 size;
    int nrChannels;
  };

  std::vector<std::unique_ptr<unsigned char[]>> textureStorages;
  std::vector<TextureDesciptor> textureDescriptors;

 protected:
  // texture size of the final texture
  glm::ivec2 textureSize;
  
  unsigned blankTextureIndex;
  unsigned missingTextureIndex;

 public:
  // GL texture name
  unsigned textureName;
};
}  // namespace sb