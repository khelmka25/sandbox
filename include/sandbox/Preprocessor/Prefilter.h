#pragma once

#include <filesystem>

namespace sb {
struct PrefilterMap {
  [[nodiscard]]
  static unsigned create(unsigned envCubemap, int size, int miplevels);

  [[nodiscard]]
  static unsigned loadFromFile(std::filesystem::path path);

  static bool saveToFile(unsigned cubemap, int size, int mipLevels, std::filesystem::path path);
};
}  // namespace sb