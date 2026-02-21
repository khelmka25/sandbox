#include "Graphics/Cubemap.h"

#include <array>
#include <format>
#include <iostream>
#include <string_view>
#include <utility>

using namespace std::literals::string_view_literals;

namespace sb {
namespace gfx {
namespace detail {
unsigned createCubemap(std::filesystem::path path, bool hdr) {
  unsigned cubemap;

  glGenTextures(1, &cubemap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

  // base filenames
  std::array<std::string_view, 6ull> faces = {
      "px"sv, "nx"sv, "py"sv, "ny"sv, "pz"sv, "nz"sv,
  };

  // extension
  const std::string_view ext = hdr ? "hdr"sv : "png"sv;

  // load all faces
  for (std::size_t i{}; i < faces.size(); i++) {
    const auto face = faces.at(i);
    const auto filename = std::format("{}.{}", face, ext);
    const auto filepath = path / filename;

    // attempt to load the filepath
    int width, height, channels;
    void* data(nullptr);
    if (hdr) {
      data = stbi_loadf(filepath.c_str(), &width, &height, &channels, 3);
    } else {
      data = stbi_load(filepath.c_str(), &width, &height, &channels, 4);
    }

    if (data == nullptr) {
      std::cout << "Cubemap texture failed to load at path: " << filepath << std::endl;
      continue;
    }

    const unsigned faceDir = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
    
    if (hdr) {
      std::cout << "Imported: " << filepath << " - " << width << 'x' << height << 'x' << channels << std::endl;
      glTexImage2D(faceDir, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
    } else {
      glTexImage2D(faceDir, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    stbi_image_free(data);
  }

  // cubemap texture parameters
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

  return cubemap;
}

unsigned createCubemapMipped(std::filesystem::path path, bool hdr, int mipLevels) {
  unsigned cubemap;

  glGenTextures(1, &cubemap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

  // base filenames
  std::array<std::string_view, 6ull> faces = {
      "px"sv, "nx"sv, "py"sv, "ny"sv, "pz"sv, "nz"sv,
  };

  // extension
  const std::string_view ext = hdr ? "hdr"sv : "png"sv;

  // load all faces
  for (int mip = 0; mip < mipLevels; mip++) {
    for (std::size_t i{}; i < faces.size(); i++) {
      const auto face = faces.at(i);
      const auto filename = std::format("mip{}-{}.{}", mip, face, ext);
      const auto filepath = path / filename;

      // attempt to load the filepath
      int width, height, channels;
      void* data(nullptr);

      if (hdr) {
        
        data = stbi_loadf(filepath.c_str(), &width, &height, &channels, 0);
      } else {
        data = stbi_load(filepath.c_str(), &width, &height, &channels, 4);
      }

      if (data == nullptr) {
        std::cout << "Cubemap texture failed to load at path: " << filepath << std::endl;
        continue;
      }

      const auto faceDir = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;

      // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
      // void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint
      // border, GLenum format, GLenum type, const void* data);
      if (hdr) {
        std::cout << "Imported: " << filepath << " - " << width << 'x' << height << 'x' << channels << std::endl;
        glTexImage2D(faceDir, mip, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
      } else {
        glTexImage2D(faceDir, mip, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
      }
      stbi_image_free(data);
    }
  }

  // cubemap texture parameters
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  // setup as mipped
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, mipLevels - 1);

  // reset the bound texture
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

  return cubemap;
}
}  // namespace detail

unsigned createHdrCubemap(std::filesystem::path path) {
  return detail::createCubemap(path, true);
}

unsigned createPngCubemap(std::filesystem::path path) {
  return detail::createCubemap(path, false);
}

unsigned createHdrCubemapMipped(std::filesystem::path path, int mipLevels) {
  return detail::createCubemapMipped(path, true, mipLevels);
}

}  // namespace gfx
}  // namespace sb