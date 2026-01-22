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
#include "Preprocessor/Common.h"
#include "Graphics/Cubemap/Cubemap.h"

namespace irradiance {
namespace detail {

// Prefilter Map
inline unsigned irradianceMap;
inline void createIrradianceMap() {
  glGenTextures(1, &irradianceMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

  for (int i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, common::resolution, common::resolution, 0, GL_RGB,
                 GL_FLOAT, nullptr);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

inline void destroyIrradianceMap() {
  glDeleteTextures(1, &irradianceMap);
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

}  // namespace detail

inline void run() {
  std::cout << "[irradiance] Running" << std::endl;
  Shader irradianceShader("assets/shaders/irradiance/vertex.glsl", "assets/shaders/irradiance/fragment.glsl");
  const glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
  unsigned envCubemap = gfx::createPngCubemap("assets/textures/input/");

  std::cout << "[irradiance] Creating Objects" << std::endl;
  common::createCube();
  common::createFrameBuffers();
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, common::resolution, common::resolution);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, common::captureRBO);
  detail::createIrradianceMap();

  std::cout << "[irradiance] Setup Shading" << std::endl;
  glUseProgram(irradianceShader.handle());
  glUniform1i(glGetUniformLocation(irradianceShader.handle(), "environmentMap"), 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

  std::cout << "[irradiance] Begin Drawing" << std::endl;
  // Main render loop
  glViewport(0, 0, common::resolution, common::resolution);
  for (unsigned int i = 0; i < 6; ++i) {
    glUniformMatrix4fv(glGetUniformLocation(irradianceShader.handle(), "view"), 1, GL_FALSE,
                       glm::value_ptr(common::captureViews[i]));
    glUniformMatrix4fv(glGetUniformLocation(irradianceShader.handle(), "projection"), 1, GL_FALSE,
                       glm::value_ptr(projection));

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                           detail::irradianceMap, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    common::drawCube();
  }

  std::cout << "[irradiance] Done Drawing" << std::endl;

  detail::exportIrradianceCubeMap(detail::irradianceMap, common::resolution, "assets/textures/irradiance/");

  detail::destroyIrradianceMap();
  common::destroyFrameBuffers();
  common::destroyCube();

  std::cout << "[irradiance] Done" << std::endl;
}
}  // namespace irradiance