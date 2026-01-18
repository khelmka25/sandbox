#include "Application/Application.h"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "Graphics/Context.h"

using namespace std::literals::string_view_literals;

Application::Application()
    : context("OpenGL Window", {640u, 360u}),
      shader("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl") {
  // generate and set up vertex array
  glGenVertexArrays(1, &vertexArrayHandle);
  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBindVertexArray.xhtml
  glBindVertexArray(vertexArrayHandle);

  // generate vertex buffer
  glGenBuffers(1, &vertexBufferHandle);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBufferHandle);

  // 1. Position attributes of triangulation data: Takes up 2 floats: x, y
  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glVertexAttribPointer.xhtml
  glVertexAttribPointer(0u, 2u, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void*)0);
  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glEnableVertexAttribArray.xhtml
  glEnableVertexArrayAttrib(vertexArrayHandle, 0u);

  // 2. Color attributes of triangulation data: Takes up 4 floats: r, g, b, a
  glVertexAttribPointer(1u, 4u, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void*)(2 * sizeof(float)));
  glEnableVertexArrayAttrib(vertexArrayHandle, 1u);

  // 3. Texture uv of triangulation data: Takes up 2 floats: u, v
  glVertexAttribPointer(2u, 2u, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void*)(6 * sizeof(float)));
  glEnableVertexArrayAttrib(vertexArrayHandle, 2u);

  // generate index element buffer
  glGenBuffers(1u, &indexBufferHandle);

  glBindVertexArray(0u);
  glBindBuffer(GL_ARRAY_BUFFER, 0u);
}

Application::~Application() noexcept {
  glDeleteBuffers(1, &vertexBufferHandle);
  glDeleteVertexArrays(1, &vertexArrayHandle);
  glDeleteBuffers(1, &indexBufferHandle);
}

void Application::build(Context* ctx) {
  glBindVertexArray(vertexArrayHandle);

  // // upload the vertex data
  // glBindBuffer(GL_ARRAY_BUFFER, vertexBufferHandle);
  // // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBufferData.xhtml
  // glBufferData(GL_ARRAY_BUFFER, ctx->vertices.size() * sizeof(Vertex),
  //              &ctx->vertices.at(0), GL_DYNAMIC_DRAW);

  // // upload the index element data
  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferHandle);
  // glBufferData(GL_ELEMENT_ARRAY_BUFFER, ctx->indices.size() * sizeof(unsigned),
  //              &ctx->indices.at(0), GL_DYNAMIC_DRAW);

  glBindVertexArray(0u);
}

void Application::draw(Context* ctx) noexcept(false) {
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawArrays.xhtml
  // void glDrawArrays( 	GLenum mode,
  // 	GLint first,
  // 	GLsizei count);

  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawElements.xhtml
  // void glDrawElements( 	GLenum mode,
  // 	GLsizei count,
  // 	GLenum type,
  // 	const void * indices);

  shader.enable();
  recomputeViewport({640.f, 360.f});

  glBindVertexArray(vertexArrayHandle);

  // // context: iterate through all of the draw commands
  // for (std::size_t i{}; i < ctx->commands.size(); i++) {
  //   const auto& cmd = ctx->commands.at(i);
  //   // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawElements.xhtml
  //   const unsigned count = (cmd.indices.end - cmd.indices.begin);
  //   GLenum const type = GL_UNSIGNED_INT;
  //   const unsigned offset = cmd.indices.begin;
  //   void* const indices = (void*)(offset * sizeof(unsigned));
  //   // different actions based on command
  //   switch (cmd.action) {
  //     case Action::kNone: {
  //       break;
  //     }
  //     case Action::kSwapPrimitivePoints: {
  //       glDrawElements(GL_POINTS, count, GL_UNSIGNED_INT, indices);
  //       break;
  //     }
  //     case Action::kSwapPrimitiveLines: {
  //       glDrawElements(GL_LINES, count, GL_UNSIGNED_INT, indices);
  //       break;
  //     }
  //     case Action::kSwapPrimitiveTriangles: {
  //       glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, indices);
  //       break;
  //     }
  //     case Action::kSwapTexture: {
  //       shader.setInt("sprite_sheet", cmd.texture);
  //       break;
  //     }
  //   }
  // }

  shader.disable();
  glBindVertexArray(0u);

  glfwSwapBuffers(context.window);
  glfwPollEvents();
}

void Application::recomputeViewport(glm::vec2 size) {
  // recompute the translation and the scaling matrix
  glm::mat4 I(1);
  // scale from [-1, 1] to [0, size]
  I = glm::scale(I, glm::vec3{2.f / size.x, 2.f / size.y, 0.f});
  // translate from [0, size] to [-size/2, size/2]
  I = glm::translate(I, glm::vec3{-size.x / 2.f, -size.y / 2.f, 0.f});

  shader.setMat4("projection"sv, I);
}

bool Application::isOpen() noexcept {
  return !glfwWindowShouldClose(context.window);
}