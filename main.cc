
#include "Application/Application.h"

int main() {
  Application app;
  while (app.isOpen()) {
    app.draw(nullptr);
  }

  return 0;
}