#pragma once

#include <glad/glad.h>
#include <stb_image.h>
#include <stb_image_write.h>

#include <array>
#include <format>
#include <iostream>
#include <string>
#include <vector>

#include "Graphics/Cubemap.h"
#include "Graphics/Shader.h"
#include "Preprocessor/Common.h"

namespace sb {
struct IrradianceMap {
  // rebuild the texture
  [[nodiscard]]
  static unsigned create(unsigned envCubemap3D, unsigned size) noexcept(false);

  // import the texture from
  [[nodiscard]]
  static unsigned loadFromDir(std::filesystem::path dirpath) noexcept(false);

  // export the texture to disk
  static bool saveToDir(unsigned cubemap, unsigned size, std::filesystem::path dirpath) noexcept(false);
};
}  // namespace sb