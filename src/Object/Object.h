#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// abstract object class
class Object {
 public:
  Object() : model(1.f) {}
  virtual void draw(class Shader* shader) = 0;

  glm::mat4 model;

 public:
  void translate(glm::vec3 amount) {
    model = glm::translate(model, amount);
  }
};