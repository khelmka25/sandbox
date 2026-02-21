#pragma once

#include <filesystem>

namespace sb {
struct EnvironmentMap {
  // export the texture to disk
  static bool saveToFile(unsigned envMap3D, std::filesystem::path filepath) noexcept(false);
  
  // import the texture from disk
  [[nodiscard]]
  static unsigned loadFromFile(std::filesystem::path filepath) noexcept(false);

  // convert a 2D hdr texture to a spherical cubemap
  [[nodiscard]]
  static unsigned create(std::filesystem::path filepath) noexcept(false);

  void destroy(unsigned envMap3D) noexcept(true);
};
}