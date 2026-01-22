#include "Application/Application.h"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <utility>

#include "Application/Context.h"
#include "Application/Event.h"

#include <argh.h>

#include "Preprocessor/Irradiance/Irradiance.h"
#include "Preprocessor/Prefilter/Prefilter.h"
#include "Preprocessor/BrdfLut/BrdfLut.h"

using namespace std::literals::string_view_literals;

Application::Application(char** argv, int argc) noexcept(true)
    : context("OpenGL Window", {256, 256}),
      camera("camera"sv, {-1, 0, 0}, {0,0,0}),
      skybox(),
      objectShader("assets/shaders/generic/vertex.glsl", "assets/shaders/generic/fragment.glsl"),
      skyboxShader("assets/shaders/skybox/vertex.glsl", "assets/shaders/skybox/fragment.glsl") {

  argh::parser cmdl(argv, argc);
  if (cmdl.operator[]({"-g"})) {
    // generate irradiance maps into the output folder
    glDisable(GL_CULL_FACE); 
    glDisable(GL_DEPTH_TEST);
    
    irradiance::run();
    prefilter::run();
    brdf_lut::run();
  }

  glEnable(GL_DEPTH_TEST);
  
  // px, nx, py, ny, pz, nz
  cubes[0].translate({+3, 0, 0});
  cubes[1].translate({-3, 0, 0});
  cubes[2].translate({0, +3, 0});
  cubes[3].translate({0, -3, 0});
  cubes[4].translate({0, 0, +3});
  cubes[5].translate({0, 0, -3});
  // red, orange, yellow, green, blue, purple
  colors[0] = glm::vec3(1.f, 0, 0);
  colors[1] = glm::vec3(1.f, 0.5f, 0);
  colors[2] = glm::vec3(1.f, 1.f, 0);
  colors[3] = glm::vec3(0, 1.f, 0);
  colors[4] = glm::vec3(0, 0.5f, 1.f);
  colors[5] = glm::vec3(0.5f, 0, 1);
}

void Application::draw() noexcept(false) {
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // draw models inside the scence
  objectShader.enable();
  auto model = glm::mat4(1.0f);
  auto view = camera.view();
  auto projection = glm::perspective(glm::radians(70.f), 16.f / 9.f, 0.1f, 100.0f);
  objectShader.setMat4("model", model);
  objectShader.setMat4("view", view);
  objectShader.setInt("skybox", 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.textureHandle());
  objectShader.setMat4("projection", projection);
  objectShader.setVec3("albedo", glm::vec3(1));
  objectShader.setFloat("metallic", 0);
  objectShader.setVec3("cameraPos", camera.position);

  // draw cube primitive
  cubePrimitive.draw(&objectShader);

  // draw in each of 6 directions:
  for (int i = 0; i < 6; i++) {
    objectShader.setFloat("metallic", 0);
    objectShader.setVec3("albedo", colors[i]);
    cubes[i].draw(&objectShader);
  }

  // change depth function so depth test passes when values are equal to depth buffer's content
  glDepthFunc(GL_LEQUAL);
  skyboxShader.enable();
  // remove translation from the view matrix
  view = glm::mat4(glm::mat3(camera.view()));
  skyboxShader.setMat4("view", view);
  skyboxShader.setMat4("projection", projection);
  skybox.draw(&skyboxShader);
  // set depth function back to default
  glDepthFunc(GL_LESS);

  glfwSwapBuffers(context.window);
  glfwPollEvents();
}

bool Application::isOpen() noexcept {
  return !glfwWindowShouldClose(context.window);
}

void Application::handleEvents() {
  // poll events
  glfwPollEvents();
  // handle events
  while (!context.eventQueue.empty()) {
    const auto event = context.eventQueue.front();
    switch (event.type) {
      case EventType::kKeyboardEvent: {
        const auto data = std::get<KeyboardEvent>(event.contents());
        handleKeyboardEvent(data);
        break;
      }
      case EventType::kMouseButtonEvent: {
        const auto data = std::get<MouseButtonEvent>(event.contents());
        handleMouseButtonEvent(data);
        break;
      }
      case EventType::kMousePositionEvent: {
        const auto data = std::get<MousePositionEvent>(event.contents());
        handlePositionEvent(data);
        break;
      }
      case EventType::kScrollEvent: {
        const auto data = std::get<ScrollEvent>(event.contents());
        handleScrollEvent(data);
        break;
      }
    }
    context.eventQueue.pop();
  }
}

void Application::handleKeyboardEvent(const KeyboardEvent& e) {
  switch (e.key) {
    case GLFW_KEY_O: {
      // open a file and import
      // ...
      break;
    }
    case GLFW_KEY_ESCAPE: {
      // close the program
      glfwSetWindowShouldClose(context.window, true);
      break;
    }
    default: {
      // forward to the camera
      camera.handleKeyboardEvent(e);
      break;
    }
  }
}

void Application::handleMouseButtonEvent(const MouseButtonEvent& e) {
  // forward to the camera
  camera.handleMouseButtonEvent(e);
}

void Application::handlePositionEvent(const MousePositionEvent& e) {
  // forward to the camera
  camera.handlePositionEvent(e);
}

void Application::handleScrollEvent(const ScrollEvent& e) {
  // forward to the camera
  camera.handleScrollEvent(e);
}

Application::~Application() noexcept(true) {
}