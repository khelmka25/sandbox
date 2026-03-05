#pragma once

#include <freetype/freetype.h>
#include <glad/glad.h>

#include <array>
#include <filesystem>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <iostream>

#include "Graphics/CharacterMetric.h"

namespace sb {
struct CharacterMetricSet {
  CharacterMetricSet(std::filesystem::path filepath, unsigned fontSize) noexcept(false) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
      throw std::runtime_error("failed to initialize FreeType library");
    }

    FT_Face face;
    if (FT_New_Face(ft, filepath.c_str(), 0, &face)) {
      throw std::runtime_error("failed to load font");
    }

    // use the font height as the size
    FT_Set_Pixel_Sizes(face, 0, fontSize);

    // fill with blank values
    std::uninitialized_fill(metrics.begin(), metrics.end(), CharacterMetric{});

    // 1. get the dimensions of all the characters
    for (int c = 32; c < 127; c++) {
      if (FT_Err_Ok != FT_Load_Char(face, c, FT_LOAD_BITMAP_METRICS_ONLY)) {
        std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
        continue;
      }
      
      // update the character metric
      CharacterMetric& metric = metrics.at(c);

      // face->glyph->metrics is in fixed point 26.6
      // shift by 6 to get the integer part
      unsigned const glyphWidth = face->glyph->metrics.width >> 6;
      unsigned const glyphHeight = face->glyph->metrics.height >> 6;
      unsigned const glyphHoriAdvance = face->glyph->metrics.horiAdvance >> 6;
      unsigned const glyphVertAdvance = face->glyph->metrics.vertAdvance >> 6;
      unsigned const glyphBearingX = face->glyph->metrics.horiBearingX >> 6;
      unsigned const glyphBearingY = face->glyph->metrics.horiBearingY >> 6;

      // advance and bearings
      metric.horiAdvance = glyphHoriAdvance;
      metric.horiBearing = glyphBearingX;
      metric.vertBearing = glyphBearingY;
      // width, height in pixels
      metric.width = glyphWidth;
      metric.height = glyphHeight;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
  }

  std::array<CharacterMetric, 128> metrics;
};

}  // namespace sb