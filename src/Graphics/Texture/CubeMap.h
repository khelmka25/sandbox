#pragma once

#include <glad/glad.h>
#include <stb_image.h>

#include <initializer_list>
#include <iostream>
#include <filesystem>

class CubeMap {
 public:
  CubeMap(std::filesystem::path t_path, bool useHdr = false) noexcept(true);
  ~CubeMap() noexcept(true);

  auto textureHandle() const noexcept { return cubemapTextureHandle; }

 protected:
  std::filesystem::path path;
  unsigned int cubemapTextureHandle;
};