#pragma once

#include <glad/glad.h>
#include <stb_image_write.h>

#include <array>
#include <format>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "Graphics/Cubemap/Cubemap.h"
#include "Graphics/Shader.h"
#include "Preprocessor/Common.h"

namespace sb {
namespace gfx {
inline unsigned createPrefilterMap(unsigned envCubemap, int size, int miplevels) {
  // pbr: setup framebuffer
  // ----------------------
  unsigned int captureFBO;
  unsigned int captureRBO;
  glGenFramebuffers(1, &captureFBO);
  glGenRenderbuffers(1, &captureRBO);

  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, std::max(size, 512), std::max(size, 512));
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

  // pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
  // --------------------------------------------------------------------------------
  unsigned int prefilterMap;
  glGenTextures(1, &prefilterMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
  for (unsigned int i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);  // be sure to set minification filter to mip_linear
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  // pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
  // ----------------------------------------------------------------------------------------------------
  glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
  Shader prefilterShader("prefilter", "assets/shaders/cubemap.vs", "assets/shaders/prefilter.fs");
  prefilterShader.use();
  prefilterShader.setInt("environmentMap", 0);
  prefilterShader.setMat4("projection", captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  unsigned int maxMipLevels = 5;
  common::createCube();
  for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
    // reisze framebuffer according to mip-level size.
    unsigned int mipWidth = (size >> mip);
    unsigned int mipHeight = (size >> mip);
    if ((mipHeight == 0) || (mipWidth == 0)) {
      std::cout << "[prefilter] Invalid mip level: " << mip << std::endl;
      continue;
    }
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
    glViewport(0, 0, mipWidth, mipHeight);

    float roughness = (float)mip / (float)(maxMipLevels - 1);
    prefilterShader.setFloat("roughness", roughness);
    for (unsigned int i = 0; i < 6; ++i) {
      prefilterShader.setMat4("view", common::captureViews[i]);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap,
                             mip);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      common::drawCube();
    }
  }
  common::destroyCube();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteRenderbuffers(1, &captureRBO);
  glDeleteFramebuffers(1, &captureFBO);

  return prefilterMap;
}

inline void exportPrefilterMap(unsigned cubemap, int size, int mipLevels, std::filesystem::path path) {
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

}  // namespace gfx
}  // namespace sb