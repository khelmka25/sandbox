#include "Graphics/TextureAtlas.h"

#include <freetype/freetype.h>
#include <glad/glad.h>
#include <rectpack2D/finders_interface.h>
#include <stb_image.h>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <utility>
#include <vector>

namespace sb {
unsigned TextureAtlas::addTexture(std::filesystem::path filepath) {
  unsigned index = textureMetadata.size();
  // load the texture and store it in memory

  TextureMetadata metadata;
  unsigned char* data = stbi_load(filepath.c_str(), &metadata.size.x, &metadata.size.y, &metadata.nrChannels, 0);
  const unsigned int size = metadata.size.x * metadata.size.y * metadata.nrChannels;

  metadata.data = std::make_unique<unsigned char[]>(size);
  memcpy(metadata.data.get(), data, size);

  stbi_image_free(data);

  textureMetadata.push_back(std::move(metadata));
  return index;
}

std::pair<glm::vec2, glm::vec2> TextureAtlas::getTextureUv(unsigned textureName) {
  const unsigned index = textureName;

  // compute the uv coordinates of the subtexture
  TextureMetadata& metadata = textureMetadata.at(index);

  // set the corners of the uvrect
  glm::vec2 p1(metadata.position);
  glm::vec2 p2(metadata.position + metadata.size);

  // normalize to [0, 1] range
  p1 = p1 / glm::vec2(textureSize);
  p2 = p2 / glm::vec2(textureSize);

  return std::make_pair(p1, p2);
}

unsigned TextureAtlas::addFont(std::filesystem::path filepath, int fontSize) {
  unsigned baseIndex = textureMetadata.size();

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

  // only load characters: space (0x20)(" ") through backtick (0x7e)("`")
  for (int c = 0x20; c < 0x7F; c++) {
    if (FT_Err_Ok != FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
      continue;
    }

    auto& metric = face->glyph->metrics;

    TextureMetadata metadata;
    // face->glyph->metrics is in fixed point 26.6
    // shift by 6 to get the integer part: in pixels
    metadata.size.x = metric.width >> 6;
    metadata.size.y = metric.height >> 6;

    unsigned size = metadata.size.y * metadata.size.x;
    metadata.data = std::make_unique<unsigned char[]>(size);
    // copy the data here
    memcpy(metadata.data.get(), face->glyph->bitmap.buffer, size);

    textureMetadata.push_back(std::move(metadata));
  }

  FT_Done_Face(face);
  FT_Done_FreeType(ft);

  return baseIndex;
}

std::pair<glm::vec2, glm::vec2> TextureAtlas::getCharacterUv(unsigned fontBaseName, int c) {
  // we don't save control characters in the texture, so give 0s
  if (std::iscntrl(c)) {
    return std::make_pair(glm::vec2(0.f), glm::vec2(0.f));
  }

  // make 0 based so space is the "first" character
  const int valid_c = c - 0x20;

  const unsigned index = fontBaseName + valid_c;

  // compute the uv coordinates of the subtexture
  TextureMetadata& metadata = textureMetadata.at(index);

  // set the corners of the uvrect
  glm::vec2 p1(metadata.position);
  glm::vec2 p2(metadata.position + metadata.size);

  // normalize to [0, 1] range
  p1 = p1 / glm::vec2(textureSize);
  p2 = p2 / glm::vec2(textureSize);

  return std::make_pair(p1, p2);
}

void TextureAtlas::recompute() {
  /// taken from: https://github.com/TeamHypersomnia/rectpack2D/blob/master/example/main.cpp

  // given a set of subtextures, compute the optimal space saving layout:
  constexpr auto allow_flip = false;
  using spaces_type = rectpack2D::empty_spaces<allow_flip, rectpack2D::default_empty_spaces>;
  using rect_type = rectpack2D::output_rect_t<spaces_type>;

  auto report_successful = [](rect_type&) { return rectpack2D::callback_result::CONTINUE_PACKING; };

  auto report_unsuccessful = [](rect_type&) { return rectpack2D::callback_result::ABORT_PACKING; };

  const auto max_side = 0x1000;
  const auto discard_step = -4;

  class my_rect {
    rect_type rect;
    int some_medadata = 0;

   public:
    my_rect(const rect_type& rect_) : rect(rect_) { (void)some_medadata; }
    auto& get_rect() { return rect; }
    const auto& get_rect() const { return rect; }
  };

  std::vector<my_rect> rectangles;

  // convert metadata into rectangles
  for (std::size_t i{}; i < textureMetadata.size(); i++) {
    const auto& metadata = textureMetadata.at(i);
    // create a rect to represent this texture 
    rect_type rect;
    rect.w - metadata.size.x;
    rect.h - metadata.size.y;
    rectangles.push_back(rect);
  }

  auto report_result = [&rectangles](const rectpack2D::rect_wh& result_size) {
    std::cout << "Resultant bin: " << result_size.w << " " << result_size.h << std::endl;

    for (const auto& rect : rectangles) {
      const auto& r = rect.get_rect();
      std::cout << r.x << " " << r.y << " " << r.w << " " << r.h << std::endl;
    }
  };

  const auto input = rectpack2D::make_finder_input(max_side, discard_step, report_successful, report_unsuccessful,
                                                   rectpack2D::flipping_option::DISABLED);

  // ensure that the order does not change                                                   
  const auto result_size = rectpack2D::find_best_packing_dont_sort<spaces_type>(rectangles, input);

  report_result(result_size);

  // now takes the result and update the metadata
  for (std::size_t i{}; i < textureMetadata.size(); i++) {
    const auto& rect = rectangles.at(i).get_rect();
    // update the metadata based on this rect
    auto& metadata = textureMetadata.at(i);
    metadata.position.x = rect.x;
    metadata.position.y = rect.y;
  }
}

void TextureAtlas::rebuffer() {
  if (textureName != 0) {
    glDeleteTextures(1, &textureName);
  }

  glGenTextures(1, &textureName);
  glBindTexture(GL_TEXTURE_2D, textureName);
  // allocate enough space for the main texture
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSize.x, textureSize.y, 0, GL_RGB, GL_FLOAT, nullptr);

  // go through all the subtextures and upload to the main texture
  for (const auto& metadata : textureMetadata) {
    const auto mip = 0;
    const auto xoffset = metadata.position.x;
    const auto yoffset = metadata.position.y;
    const auto width = metadata.size.x;
    const auto height = metadata.size.y;
    int format;
    switch (metadata.nrChannels) {
      case 4:
        format = GL_RGBA;
        break;
      case 3:
        format = GL_RGB;
        break;
      case 1:
        format = GL_RGB;
    }
    glTexSubImage2D(GL_TEXTURE_2D, mip, xoffset, yoffset, width, height, format, GL_UNSIGNED_BYTE, metadata.data.get());
  }

  // done!
}

}  // namespace sb