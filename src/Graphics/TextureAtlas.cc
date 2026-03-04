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

unsigned TextureAtlas::addBlankTexture() {
  blankTextureIndex = textureDescriptors.size();
  // make a 1x1 white, opaque pixel for use with widgets
  TextureDesciptor descriptor;
  descriptor.size.x = 1;
  descriptor.size.y = 1;
  descriptor.nrChannels = 4;
  textureDescriptors.emplace_back(std::move(descriptor));

  // white opaque: 0xffffffff
  auto storage = std::make_unique<unsigned char[]>(4);
  storage[0] = 0xff;
  storage[1] = 0xff;
  storage[2] = 0xff;
  storage[3] = 0xff;
  textureStorages.emplace_back(std::move(storage));
  return blankTextureIndex;
}

unsigned TextureAtlas::addMissingTexture() {
  missingTextureIndex = textureDescriptors.size();
  // make a patterned rect that is the following:
  // checkered black and pink color that is 4x4
  TextureDesciptor descriptor;
  descriptor.size.x = 4;
  descriptor.size.y = 4;
  descriptor.nrChannels = 3;
  textureDescriptors.emplace_back(std::move(descriptor));

  // pink: 255,0,220
  // black: 1, 0, 1
  const int size = 4 * 4;
  auto storage = std::make_unique<unsigned char[]>(size * 3);
  for (int i = 0; i < size; i++) {
    // if odd: use black tile
    if (i & 1) {
      storage[i * 3 + 0] = 1;
      storage[i * 3 + 1] = 0;
      storage[i * 3 + 2] = 1;
    } else {
      storage[i * 3 + 0] = 255;
      storage[i * 3 + 1] = 0;
      storage[i * 3 + 2] = 220;
    }
  }
  textureStorages.emplace_back(std::move(storage));

  return missingTextureIndex;
}

unsigned TextureAtlas::addTexture(std::filesystem::path filepath) noexcept(false) {
  const unsigned index = textureDescriptors.size();
  // load the texture and store it in memory
  TextureDesciptor descriptor;
  unsigned char* data = stbi_load(filepath.c_str(), &descriptor.size.x, &descriptor.size.y, &descriptor.nrChannels, 0);
  const unsigned int size = descriptor.size.x * descriptor.size.y * descriptor.nrChannels;
  textureDescriptors.push_back(std::move(descriptor));

  auto storage = std::make_unique<unsigned char[]>(size);
  // copy the bytes data
  for (unsigned i = 0; i < size; i++) {
    storage[i] = data[i];
  }

  textureStorages.emplace_back(std::move(storage));
  stbi_image_free(data);
  return index;
}

std::pair<glm::vec2, glm::vec2> TextureAtlas::getTextureUv(unsigned textureName) noexcept(false)  {
  const unsigned index = textureName;

  // compute the uv coordinates of the subtexture
  TextureDesciptor& descriptor = textureDescriptors.at(index);

  // set the corners of the uvrect
  glm::vec2 p1(descriptor.position);
  glm::vec2 p2(descriptor.position + descriptor.size);

  // normalize to [0, 1] range
  p1 = p1 / glm::vec2(textureSize);
  p2 = p2 / glm::vec2(textureSize);

  return std::make_pair(p1, p2);
}

unsigned TextureAtlas::addFont(std::filesystem::path filepath, int fontSize) noexcept(false) {
  const unsigned index = textureDescriptors.size();

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

    TextureDesciptor descriptor;
    // face->glyph->metrics is in fixed point 26.6
    // shift by 6 to get the integer part: in pixels
    descriptor.size.x = metric.width >> 6;
    descriptor.size.y = metric.height >> 6;
    descriptor.nrChannels = 4;
    textureDescriptors.emplace_back(std::move(descriptor));

    const unsigned size = descriptor.size.y * descriptor.size.x * 4;
    // convert source from Rs to dest RGBAd (with RGBd = 0xff, and Ad = Rs)
    auto storage = std::make_unique<unsigned char[]>(size);
    for (int row = 0; row < descriptor.size.y; row++) {
      for (int col = 0; col < descriptor.size.x; col++) {
        const int index = (row * descriptor.size.x + col);
        storage[index * 4 + 0] = 0xff;
        storage[index * 4 + 1] = 0xff;
        storage[index * 4 + 2] = 0xff;

        storage[index * 4 + 3] = face->glyph->bitmap.buffer[index];
      }
    }

    textureStorages.emplace_back(std::move(storage));
  }

  FT_Done_Face(face);
  FT_Done_FreeType(ft);

  return index;
}

std::pair<glm::vec2, glm::vec2> TextureAtlas::getCharacterUv(unsigned fontBaseName, int c) noexcept(false)  {
  // we don't save control characters in the texture, so give 0s
  if (std::iscntrl(c)) {
    return std::make_pair(glm::vec2(0.f), glm::vec2(0.f));
  }

  // make 0 based so space is the "first" character
  const int valid_c = c - 0x20;

  const unsigned index = fontBaseName + valid_c;

  // compute the uv coordinates of the subtexture
  TextureDesciptor& descriptor = textureDescriptors.at(index);

  // set the corners of the uvrect
  glm::vec2 p1(descriptor.position);
  glm::vec2 p2(descriptor.position + descriptor.size);

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

  // convert descriptor into rectangles
  for (std::size_t i{}; i < textureDescriptors.size(); i++) {
    const auto& descriptor = textureDescriptors.at(i);
    // create a rect to represent this texture 
    rect_type rect;
    rect.w = descriptor.size.x;
    rect.h = descriptor.size.y;
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

  // report_result(result_size);

  // now takes the result and update the descriptor
  for (std::size_t i{}; i < textureDescriptors.size(); i++) {
    const auto& rect = rectangles.at(i).get_rect();
    // update the descriptor based on this rect
    auto& descriptor = textureDescriptors.at(i);
    descriptor.position.x = rect.x;
    descriptor.position.y = rect.y;
  }

  textureSize.x = result_size.w;
  textureSize.y = result_size.h;
}

void TextureAtlas::rebuffer() {
  if (textureName != 0) {
    glDeleteTextures(1, &textureName);
  }

  glGenTextures(1, &textureName);
  glBindTexture(GL_TEXTURE_2D, textureName);
  // allocate enough space for the main texture
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSize.x, textureSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  // set texture options
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  // can either make nearest or linear?
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // go through all the subtextures and upload to the main texture
  for (std::size_t i{}; i < textureDescriptors.size(); i++) {
    const auto& descriptor = textureDescriptors.at(i);
    const auto& storage = textureStorages.at(i);

    const auto mip = 0;
    const auto xoffset = descriptor.position.x;
    const auto yoffset = descriptor.position.y;
    const auto width = descriptor.size.x;
    const auto height = descriptor.size.y;
    int format;
    switch (descriptor.nrChannels) {
      case 4:
        format = GL_RGBA;
        break;
      case 3:
        format = GL_RGB;
        break;
      case 1:
        format = GL_RGB;
    }
    glTexSubImage2D(GL_TEXTURE_2D, mip, xoffset, yoffset, width, height, format, GL_UNSIGNED_BYTE, storage.get());
  }

  glGenerateMipmap(GL_TEXTURE_2D);
  // done!
}

}  // namespace sb