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
  // add a single texture, and return the referencing name
  [[nodiscard]]
  unsigned addTexture(std::filesystem::path filepath);

  [[nodiscard]]
  std::pair<glm::vec2, glm::vec2> getTextureUv(unsigned textureName);

  // add a font (multiple textures) and return the referencing name
  [[nodiscard]]
  unsigned addFont(std::filesystem::path filepath, int size);

  [[nodiscard]]
  std::pair<glm::vec2, glm::vec2> getCharacterUv(unsigned fontBaseName, int c);

  // recompute the locations of the subtextures
  void recompute();

  // upload the finished texture to the gpu
  // based on the recomputed locations, create a texture on the gpu
  // and upload all subtextures into this texture
  void rebuffer();

  struct TextureMetadata {
    glm::ivec2 position;
    glm::ivec2 size;
    int nrChannels;
    std::unique_ptr<unsigned char[]> data;
  };

  std::vector<TextureMetadata> textureMetadata;

 protected:
  // texture size of the final texture
  glm::ivec2 textureSize;

  // GL texture name
  unsigned textureName;
};
}  // namespace sb