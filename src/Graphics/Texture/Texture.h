#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "Graphics/Texture/TextureDescriptor.h"

struct TextureOpenGL {
  TextureOpenGL(unsigned t_width, unsigned t_height, unsigned t_channels, char* data)
      : width(t_width), height(t_height), channels(t_channels) {
    glGenTextures(1, &nativeTextureHandle);
    glBindTexture(GL_TEXTURE_2D, nativeTextureHandle);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }

  TextureDescriptor getTextureDescriptor() const noexcept {
    TextureDescriptor out;
    out.width = width;
    out.height = height;
    return out;
  }

  ~TextureOpenGL() noexcept {
    glDeleteTextures(1, &nativeTextureHandle);
  }

  auto getNativeHandle() const noexcept -> unsigned {
    return nativeTextureHandle;
  }

  unsigned width = 0u;
  unsigned height = 0u;
  unsigned channels = 0u;
  unsigned nativeTextureHandle = 0u;
};