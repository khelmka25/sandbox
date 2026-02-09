#pragma once

#include <glad/glad.h>
#include <stb_image.h>
#include <stb_image_write.h>

#include <array>
#include <format>
#include <iostream>
#include <string>
#include <vector>

#include "Graphics/Cubemap/Cubemap.h"
#include "Graphics/Shader.h"
#include "Preprocessor/Common.h"

namespace sb {
namespace gfx {
inline unsigned createIrradianceMap(unsigned envCubemap, int size) {
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

  // pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
  // --------------------------------------------------------------------------------
  unsigned int irradianceMap;
  glGenTextures(1, &irradianceMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
  for (unsigned int i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);

  // pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
  // -----------------------------------------------------------------------------
  glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
  Shader irradianceShader("irradiance", "assets/shaders/cubemap.vs", "assets/shaders/irradiance.fs");
  irradianceShader.use();
  irradianceShader.setInt("environmentMap", 0);
  irradianceShader.setMat4("projection", captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

  glViewport(0, 0, size, size);  // don't forget to configure the viewport to the capture dimensions.
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  common::createCube();
  for (unsigned int i = 0; i < 6; ++i) {
    irradianceShader.setMat4("view", common::captureViews[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    common::drawCube();
  }
  common::destroyCube();
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteRenderbuffers(1, &captureRBO);
  glDeleteFramebuffers(1, &captureFBO);

  return irradianceMap;
}

inline void exportIrradianceCubeMap(unsigned cubemap, int size, std::filesystem::path path) {
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

  std::vector<float> pixels(size * size * 3);

  std::array<const char*, 6ull> faces = {"px", "nx", "py", "ny", "pz", "nz"};

  for (int i = 0; i < 6; ++i) {
    glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, GL_FLOAT, pixels.data());

    auto filename = std::format("{}.hdr", faces[i]);
    auto filepath = path / filename;

    if (!stbi_write_hdr(filepath.c_str(), size, size, 3, pixels.data())) {
      std::cerr << "Failed to write " << filepath << "\n";
    } else {
      std::cout << "Exported: " << filepath << " - " << size << 'x' << size << 'x' << 3 << std::endl;
    }
  }
}

}  // namespace gfx
}  // namespace sb