#pragma once

#include <glad/glad.h>

#include <filesystem>

#include "Graphics/Texture/TextureDescriptor.h"

struct Texture {
  explicit Texture(std::filesystem::path t_path);

  TextureDescriptor getTextureDescriptor() const noexcept {
    TextureDescriptor out;
    out.width = width;
    out.height = height;
    return out;
  }

  ~Texture() noexcept {
    glDeleteTextures(1, &textureHandle);
  }

  auto handle() const noexcept -> unsigned {
    return textureHandle;
  }

  signed width = 0u;
  signed height = 0u;
  signed channels = 0u;
  unsigned textureHandle = 0u;
};