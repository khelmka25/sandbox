#pragma once

#include <freetype/freetype.h>
#include <glad/glad.h>

#include <array>
#include <filesystem>
#include <glm/vec2.hpp>
#include <iostream>
#include <unordered_map>

struct CharacterMetric {
  // unit coordinates
  struct UvRect {
    glm::vec2 uv0;
    glm::vec2 uv1;
  } uv;

  // save the following:
  float advance_x;

  // width of the glyph in pixels
  float width;
  float height;

  float bearingY;
  float bearingX;
};

struct Atlas {
  Atlas(std::filesystem::path filepath) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
      std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
      return;
    }

    FT_Face face;
    if (FT_New_Face(ft, filepath.c_str(), 0, &face)) {
      std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
      return;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    // generate an empty texture to be filled

    // 1. get the dimensions of all the characters
    unsigned rows = 0;
    unsigned width = 0;
    for (int c = 32; c < 127; c++) {
      if (FT_Err_Ok != FT_Load_Char(face, c, FT_LOAD_BITMAP_METRICS_ONLY)) {
        std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
        continue;
      }

      rows = std::max(rows, face->glyph->bitmap.rows);
      width = width + face->glyph->bitmap.width;
      // create an entry for the character:
      std::cout << c << ": " << face->glyph->bitmap.width << 'x' << face->glyph->bitmap.rows << std::endl;
    }

    std::cout << "rows: " << rows << std::endl;
    std::cout << "width: " << width << std::endl;

    // 2. create an empty GL texture with the width and rows
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, rows, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
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

      // yoffset = face->glyph->metrics.width;
      glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RED,
                      GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

      xoffset = xoffset + face->glyph->bitmap.width;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
  }

  // texture object
  unsigned texture;
};