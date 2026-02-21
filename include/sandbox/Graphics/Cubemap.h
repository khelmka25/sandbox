#pragma once

#include <glad/glad.h>
#include <stb_image.h>

#include <filesystem>
#include <initializer_list>
#include <iostream>

namespace sb {
namespace gfx {
namespace detail {
extern unsigned createCubemap(std::filesystem::path path, bool hdr);
extern unsigned createCubemapMipped(std::filesystem::path path, bool hdr, int mipLevels);
}  // namespace detail

extern unsigned createHdrCubemap(std::filesystem::path path);
extern unsigned createPngCubemap(std::filesystem::path path);
extern unsigned createHdrCubemapMipped(std::filesystem::path path, int mipLevels);
}  // namespace gfx
}  // namespace sb