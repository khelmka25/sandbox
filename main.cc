
#include <glad/glad.h>

#include "Application/Application.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

int main(int argc, char** argv) {
  Application app(argv, argc);
  while (app.isOpen()) {
    app.handleEvents();
    app.draw();
  }

  return 0;
}