#pragma once

#include <glad/glad.h>

#include <filesystem>

namespace gfx {
namespace detail {
extern unsigned createTexture(std::filesystem::path path, bool hdr);
}
extern unsigned createHdrTexture(std::filesystem::path path);
extern unsigned createPngTexture(std::filesystem::path path);
};