#pragma once

#include <glad/glad.h>
#include <stb_image.h>
#include <stb_image_write.h>

#include <array>
#include <format>
#include <iostream>
#include <string>
#include <vector>

#include "Graphics/Shader.h"
#include "Graphics/Texture/CubeMap.h"
#include "Preprocessor/Common.h"

namespace brdf_lut {
namespace detail {
inline const unsigned resolution = 512;

inline unsigned brdfLutTexture;
inline void createBrdfLut() {
  glGenTextures(1, &brdfLutTexture);
  glBindTexture(GL_TEXTURE_2D, brdfLutTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, resolution, resolution, 0, GL_RG, GL_FLOAT, nullptr);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

inline void destroyBrdfLut() {
  glDeleteTextures(1, &brdfLutTexture);
}

inline unsigned captureFBO;
inline unsigned captureRBO;
inline void createFramebuffer() {
  glGenFramebuffers(1, &captureFBO);
  glGenRenderbuffers(1, &captureRBO);

  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, resolution, resolution);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
}

inline void destroyFrameBuffers() {
  glDeleteRenderbuffers(1, &captureRBO);
  glDeleteFramebuffers(1, &captureFBO);
}

inline void exportBrdfLut(unsigned texture, unsigned size, std::filesystem::path path) {
  glBindTexture(GL_TEXTURE_2D, texture);

  std::vector<float> pixels(size * size * 2);

  glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, pixels.data());

  auto filename = std::format("{}.hdr", "brdf-lut");
  auto filepath = path / filename;

  if (!stbi_write_hdr(filepath.c_str(), size, size, 2, pixels.data())) {
    std::cerr << "Failed to write " << filepath << "\n";
  } else {
    std::cout << "Exported: " << filepath << std::endl;
  }
}

}  // namespace detail

inline void run() {
  std::cout << "[brdf-lut] Running" << std::endl;
  Shader brdfShader("assets/shaders/brdf-lut/vertex.glsl", "assets/shaders/brdf-lut/fragment.glsl");

  std::cout << "[brdf-lut] Creating Objects" << std::endl;
  common::createQuad();
  detail::createBrdfLut();
  detail::createFramebuffer();

  std::cout << "[brdf-lut] Setup Shading" << std::endl;
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, detail::brdfLutTexture, 0);

  glViewport(0, 0, detail::resolution, detail::resolution);

  std::cout << "[brdf-lut] Begin Drawing" << std::endl;
  glUseProgram(brdfShader.handle());

  common::drawQuad();
  std::cout << "[brdf-lut] Done Drawing" << std::endl;

  detail::exportBrdfLut(detail::brdfLutTexture, detail::resolution, "assets/textures/brdf-lut/");

  detail::destroyFrameBuffers();
  detail::destroyBrdfLut();
  common::destroyQuad();

  std::cout << "[brdf-lut] Done" << std::endl;
}

}  // namespace brdf_lut