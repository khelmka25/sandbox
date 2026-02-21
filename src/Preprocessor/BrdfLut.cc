#include "Preprocessor/BrdfLut.h"

#include <glad/glad.h>
#include <stb_image.h>
#include <stb_image_write.h>

#include <array>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "Graphics/Shader.h"
#include "Preprocessor/Common.h"

namespace sb {
// import the texture from disk
unsigned BrdfLut::loadFromFile(std::filesystem::path filepath) noexcept(false) {
  int width, height, channelCount;

  float* data = stbi_loadf(filepath.c_str(), &width, &height, &channelCount, 0);

  if (!data) {
    std::cerr << "Texture failed to load at path: " << filepath << std::endl;
    return 0;
  }

  return 0;
}

// export the texture to disk
bool BrdfLut::saveToFile(unsigned texture, unsigned size, std::filesystem::path filepath) noexcept(false) {
  if (std::cmp_equal(texture, 0)) {
    return false;
  }

  glBindTexture(GL_TEXTURE_2D, texture);

  std::vector<float> pixels(size * size * 2);

  glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, pixels.data());

  if (!stbi_write_hdr(filepath.c_str(), size, size, 2, pixels.data())) {
    std::cerr << "Failed to write " << filepath << "\n";
    return false;
  } else {
    std::cout << "Exported: " << filepath << " - " << size << 'x' << size << 'x' << 2 << std::endl;
  }

  return true;
}

// rebuild the texture
unsigned BrdfLut::create(unsigned size) noexcept(false) {
  // pbr: setup framebuffer
  // ----------------------
  unsigned int captureFBO;
  unsigned int captureRBO;
  glGenFramebuffers(1, &captureFBO);
  glGenRenderbuffers(1, &captureRBO);

  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, std::max(size, 512u), std::max(size, 512u));
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

  // pbr: generate a 2D LUT from the BRDF equations used.
  // ----------------------------------------------------
  Shader brdfShader("brdf", "assets/shaders/preprocessor/brdf.vs", "assets/shaders/preprocessor/brdf.fs");
  unsigned texture;
  glGenTextures(1, &texture);

  // pre-allocate enough memory for the LUT texture.
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, size, size, 0, GL_RG, GL_FLOAT, 0);
  // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

  common::createQuad();
  glViewport(0, 0, size, size);
  brdfShader.use();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  common::drawQuad();

  common::destroyQuad();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteRenderbuffers(1, &captureRBO);
  glDeleteFramebuffers(1, &captureFBO);

  return texture;
}
}  // namespace sb
