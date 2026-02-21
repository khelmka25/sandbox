#pragma once

#include <filesystem>

namespace sb {
struct BrdfLut {
  // rebuild the texture
  [[nodiscard]]
  static unsigned create(unsigned size) noexcept(false);

  // export the texture to disk
  static bool saveToFile(unsigned texture, unsigned size, std::filesystem::path filepath) noexcept(false);

  // import the texture from disk
  [[nodiscard]]
  static unsigned loadFromFile(std::filesystem::path filepath) noexcept(false);
};

}  // namespace sb