#include "Graphics/Texture/CubeMap.h"

#include <array>
#include <string_view>

using namespace std::literals::string_view_literals;

CubeMap::CubeMap(std::filesystem::path t_path, bool useHdr) noexcept(true) : path(t_path) {
  if (useHdr) {
    std::array<std::string_view, 6ull> files({
        "px.hdr"sv,
        "nx.hdr"sv,
        "py.hdr"sv,
        "ny.hdr"sv,
        "pz.hdr"sv,
        "nz.hdr"sv,
    });

    // create the cubemap texture handle
    glGenTextures(1, &cubemapTextureHandle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureHandle);

    for (std::size_t i{}; i < files.size(); i++) {
      const auto file = files.at(i);
      auto filepath = path / file;
      // attempt to load the filepath
      int width, height, channels;
      float* data = stbi_loadf(filepath.c_str(), &width, &height, &channels, 0);
      if (data == nullptr) {
        std::cout << "Cubemap texture failed to load at path: " << filepath << std::endl;
        stbi_image_free(data);
      } else {
        const auto faceDir = GL_TEXTURE_CUBE_MAP_POSITIVE_X + unsigned(i);
        glTexImage2D(faceDir, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
        stbi_image_free(data);
      }
    }

    // cubemap texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  } else {
    // attempt to load all the files from the directory:
    std::array<std::string_view, 6ull> files({
        "px.png"sv,
        "nx.png"sv,
        "py.png"sv,
        "ny.png"sv,
        "pz.png"sv,
        "nz.png"sv,
    });

    // create the cubemap texture handle
    glGenTextures(1, &cubemapTextureHandle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureHandle);

    for (std::size_t i{}; i < files.size(); i++) {
      const auto file = files.at(i);
      auto filepath = path / file;
      // attempt to load the filepath
      int width, height, channels;
      unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
      if (data == nullptr) {
        std::cout << "Cubemap texture failed to load at path: " << filepath << std::endl;
        stbi_image_free(data);
      } else {
        const auto faceDir = GL_TEXTURE_CUBE_MAP_POSITIVE_X + unsigned(i);
        glTexImage2D(faceDir, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
      }
    }

    // cubemap texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  }
}

CubeMap::~CubeMap() {
  glDeleteTextures(1, &cubemapTextureHandle);
}