#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace sb {
// abstract object class
class Object {
 public:
  Object(glm::vec3 t_center) : center(t_center) {
    modelMatrix = glm::mat4(1.f);
    modelMatrix = glm::translate(modelMatrix, center);
  }

  // void moveTo(glm::vec3 t_position) {
  //   position = t_position;
  // }

  // void rotateTo(float angle, glm::vec3 axis) {

  // }

  virtual void draw(class Shader* shader) = 0;

  // required:
  // translation
  // rotation about object's center
  // rotation about external point

  void translateBy(glm::vec3 amount) {
    center = center + amount;
    recomputeInternals();
  }

  void rotateAround(glm::vec3 amount, glm::vec3 point, float angle, glm::vec3 axis) {
    // apply the rotation about the point: rotate the object about its center
    glm::mat4 model(1);
    model = glm::translate(model, -center);
    model = glm::rotate(model, angle, axis);
    // model = glm::translate(model, center);

    // rotate the center about the point: translate to this new point
    glm::vec4 center4(center, 1);
    glm::mat4 model2(1);
    model2 = glm::rotate(model2, angle, axis);
    glm::vec3 newCenter = glm::vec3(model2 * center4);
    // auto offset = newCenter - center;
    model = glm::translate(model, newCenter);

    modelMatrix = model;
  }

  // rotate about origin (position)
  void rotateBy(glm::vec3 amount) {
    rotation = rotation + amount;
    recomputeInternals();
  }

  // cached model matrix
  glm::mat4 modelMatrix;

 protected:
  glm::vec3 center;
  glm::vec3 rotation;

  void recomputeInternals() {}
};
}  // namespace sb