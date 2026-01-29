#include "Graphics/Texture/Texture.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>

namespace sb {
namespace gfx {
namespace detail {
unsigned createTexture(std::filesystem::path path, bool hdr) {
  int width, height, channels;
  void* data(nullptr);
  
  if (hdr) {
    data = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
  } else {
    data = stbi_load(path.c_str(), &width, &height, &channels, 4);
  }

  if (data == nullptr) {
    std::cerr << "Texture failed to load at path: " << path << std::endl;
    return 0;
  }

  // get the format
  const auto format = [](signed nChannels) {
    switch (nChannels) {
      default:
      case 1: {
        return GL_RED;
      }
      case 3: {
        return GL_RGB;
      }
      case 4: {
        return GL_RGBA;
      }
    }
  }(channels);

  unsigned texture;
  // generate a new texture texture on the gpu
  glGenTextures(1, &texture);

  // set up the texture
  glBindTexture(GL_TEXTURE_2D, texture);
  // upload the texture data
  if (hdr) {
    std::cout << "Imported: " << path << " - " << width << 'x' << height << std::endl;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_FLOAT, data);
  } else {
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
  }
  
  glGenerateMipmap(GL_TEXTURE_2D);

  // set the texture wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindTexture(GL_TEXTURE_2D, 0);

  stbi_image_free(data);

  return texture;
}
}  // namespace detail

unsigned createHdrTexture(std::filesystem::path path) {
  return detail::createTexture(path, true);
}

unsigned createPngTexture(std::filesystem::path path) {
  return detail::createTexture(path, true);
}
};  // namespace gfx
}  // namespace sb