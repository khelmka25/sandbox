#include "Graphics/Texture/Texture.h"

#include <iostream>

#include <stb_image.h>

Texture::Texture(std::filesystem::path t_filepath) noexcept(false) {
  unsigned char* data = stbi_load(t_filepath.c_str(), &this->width, &this->height, &this->channels, 0);
  if (!data) {
    std::cerr << "Texture failed to load at path: " << t_filepath << std::endl;
    stbi_image_free(data);
    throw std::runtime_error("filed to load file");
  }

  // get the format
  const GLenum format = [](signed nChannels) {
    switch (nChannels) {
      case 1: {
        return GL_RED;
      }
      case 3: {
        return GL_RGB;
      }
      case 4: {
        return GL_RGBA;
      }
      default: {
        throw std::runtime_error("Unkown format");
      }
    }
  } (this->channels);

  // generate a new texture textureHandle on the gpu
  unsigned int textureHandle;
  glGenTextures(1, &textureHandle);

  // set up the texture
  glBindTexture(GL_TEXTURE_2D, textureHandle);
  // upload the texture data
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  // set the texture wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  stbi_image_free(data);
}