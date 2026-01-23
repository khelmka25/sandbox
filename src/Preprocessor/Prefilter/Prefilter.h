#pragma once

#include <glad/glad.h>
#include <stb_image_write.h>

#include <array>
#include <format>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "Graphics/Shader.h"
#include "Graphics/Cubemap/Cubemap.h"
#include "Preprocessor/Common.h"

namespace prefilter {
namespace detail {
inline const int maxMipLevels = 5;

// Prefilter Map
inline unsigned prefilterMap;
inline void createPrefilterMap() {
  glGenTextures(1, &prefilterMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);

  for (int mip = 0; mip < maxMipLevels; mip++) {
    unsigned int mipSize = common::resolution >> mip;
    for (int i = 0; i < 6; i++) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mip, GL_RGB16F, mipSize, mipSize, 0, GL_RGB, GL_FLOAT, nullptr);
    }
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, detail::maxMipLevels - 1);
}

inline void destroyPrefilterMap() {
  glDeleteTextures(1, &prefilterMap);
}

inline void exportPrefilterCubeMap(unsigned cubemap, int size, int mipLevels, std::filesystem::path path) {
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

  using namespace std::string_view_literals;
  std::array<std::string_view, 6ull> faces = {"px"sv, "nx"sv, "py"sv, "ny"sv, "pz"sv, "nz"sv};

  for (int mip = 0; mip < mipLevels; mip++) {
    unsigned int mipWidth = size >> mip;
    unsigned int mipHeight = size >> mip;

    std::vector<float> pixels(mipWidth * mipHeight * 3);

    for (int i = 0; i < 6; ++i) {
      glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mip, GL_RGB, GL_FLOAT, pixels.data());

      auto filename = std::format("mip{}-{}.hdr", mip, faces[i]);
      auto filepath = path / filename;

      if (!stbi_write_hdr(filepath.c_str(), mipWidth, mipHeight, 3, pixels.data())) {
        std::cerr << "Failed to write " << filepath << "\n";
      } else {
        std::cout << "Exported: " << filepath << " - " << size << 'x' << size << 'x' << 3 << std::endl;
      }
    }
  }
}
}  // namespace detail

inline unsigned run() {
  std::cout << "[prefilter] Running" << std::endl;
  Shader prefilterShader("assets/shaders/prefilter/vertex.glsl", "assets/shaders/prefilter/fragment.glsl");
  unsigned envCubemap = gfx::createHdrCubemap("assets/textures/input/");

  std::cout << "[prefilter] Creating Objects" << std::endl;
  common::createCube();
  common::createFrameBuffers();
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, common::resolution, common::resolution);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, common::captureRBO);

  detail::createPrefilterMap();
  
  std::cout << "[prefilter] Setup Shading" << std::endl;
  glUseProgram(prefilterShader.handle());
  glUniform1i(glGetUniformLocation(prefilterShader.handle(), "environmentMap"), 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

  std::cout << "[prefilter] Begin Drawing" << std::endl;
  // Main render loop
  for (unsigned int mip = 0; mip < detail::maxMipLevels; mip++) {
    unsigned int mipWidth = common::resolution >> mip;
    unsigned int mipHeight = common::resolution >> mip;

    glBindRenderbuffer(GL_RENDERBUFFER, common::captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);

    glViewport(0, 0, mipWidth, mipHeight);

    float roughness = (float)mip / (float)(detail::maxMipLevels - 1);
    // std::cout << "[prefilter] mip: " << mip << ", roughness: " << roughness << std::endl;
    glUniform1f(glGetUniformLocation(prefilterShader.handle(), "roughness"), roughness);

    for (unsigned int i = 0; i < 6; i++) {
      glUniformMatrix4fv(glGetUniformLocation(prefilterShader.handle(), "view"), 1, GL_FALSE,
                         glm::value_ptr(common::captureViews[i]));

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             detail::prefilterMap, mip);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      common::drawCube();
    }
  }

  std::cout << "[prefilter] Done Drawing" << std::endl;

  detail::exportPrefilterCubeMap(detail::prefilterMap, common::resolution, detail::maxMipLevels,
                                 "assets/textures/prefilter");

  // detail::destroyPrefilterMap();
  common::destroyFrameBuffers();
  common::destroyCube();

  std::cout << "[prefilter] Done" << std::endl;

  return detail::prefilterMap;
}
}  // namespace prefilter