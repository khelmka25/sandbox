#pragma once

#include <freetype/freetype.h>
#include <glad/glad.h>

#include <array>
#include <filesystem>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <iostream>

#include "Graphics/Text/CharacterMetric.h"

namespace sb {
struct Atlas {
  Atlas(std::filesystem::path filepath, unsigned fontSize) noexcept(false) {
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

    // 1. get the dimensions of all the characters
    unsigned textureWidth = 0;
    unsigned textureHeight = 0;
    for (int c = 32; c < 127; c++) {
      if (FT_Err_Ok != FT_Load_Char(face, c, FT_LOAD_BITMAP_METRICS_ONLY)) {
        std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
        continue;
      }

      auto& metric = face->glyph->metrics;

      // face->glyph->metrics is in fixed point 26.6
      // shift by 6 to get the integer part: in pixels
      unsigned const glyphWidth = metric.width >> 6;
      unsigned const glyphHeight = metric.height >> 6;

      // update the max width and height for the final texture
      textureWidth = textureWidth + glyphWidth;
      textureHeight = std::max(textureHeight, glyphHeight);
    }

    // 2. create an empty GL texture with the width and rows
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, textureWidth, textureHeight, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    // set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 3. load the bitmap image data and buffer into the GL texture
    unsigned xoffset = 0;
    unsigned yoffset = 0;
    for (int c = 32; c < 127; c++) {
      if (FT_Err_Ok != FT_Load_Char(face, c, FT_LOAD_RENDER)) {
        std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
        continue;
      }

      // face->glyph->metrics is in fixed point 26.6
      // shift by 6 to get the integer part
      unsigned const glyphWidth = face->glyph->metrics.width >> 6;
      unsigned const glyphHeight = face->glyph->metrics.height >> 6;
      unsigned const glyphHoriAdvance = face->glyph->metrics.horiAdvance >> 6;
      unsigned const glyphVertAdvance = face->glyph->metrics.vertAdvance >> 6;
      unsigned const glyphBearingX = face->glyph->metrics.horiBearingX >> 6;
      unsigned const glyphBearingY = face->glyph->metrics.horiBearingY >> 6;

      // update the character metric
      auto& metric = metrics.at(c);
      // textures uvs for sampler2D
      metric.uv_tr.x = float(xoffset) / float(textureWidth);
      metric.uv_bl.x = float(xoffset + glyphWidth) / float(textureWidth);
      metric.uv_tr.y = float(yoffset) / float(textureHeight);
      metric.uv_bl.y = float(yoffset + glyphHeight) / float(textureHeight);
      // advance and bearings
      metric.horiAdvance = glyphHoriAdvance;
      metric.horiBearing = glyphBearingX;
      metric.vertBearing = glyphBearingY;
      // width, height in pixels
      metric.width = glyphWidth;
      metric.height = glyphHeight;

      glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RED,
                      GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

      // update the cursor
      xoffset = xoffset + glyphWidth;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
  }

  unsigned texture;
  std::array<CharacterMetric, 128> metrics;
};

}  // namespace sb