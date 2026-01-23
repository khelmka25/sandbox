

#include <argh.h>

#include "Camera/Camera.h"
#include "Application/Window.h"
#include "Preprocessor/BrdfLut/BrdfLut.h"
#include "Preprocessor/Irradiance/Irradiance.h"
#include "Preprocessor/Prefilter/Prefilter.h"
#include "Object/CubePrimitive.h"
#include "Graphics/Texture/Texture.h"

using namespace std::string_view_literals;

int main(int argc, char** argv) {
  const auto title = "OpenGL Window"sv;
  const auto width = 640u;
  const auto height = 360u;
  GLFWwindow* window = gfx::createGLFWwindow(title, width, height);

  Camera camera("camera_1", {0, 0, 3}, {0, 0, 0});

  // Parse the command line, generate new textures if req
  argh::parser cmdl(argv, argc);
  if (cmdl.operator[]({"-g"})) {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    irradiance::run();
    prefilter::run();
    brdf_lut::run();
  }

  glEnable(GL_DEPTH_TEST);

  // Shaders: cubemap skybox shader
  Shader skyboxShader("assets/shaders/skybox/vertex.glsl", "assets/shaders/skybox/fragment.glsl");
  // Shaders: pbr + ibl shader
  Shader objectShader("assets/shaders/object/vertex.glsl", "assets/shaders/object/fragment.glsl");

  // Object: skybox
  CubePrimitive skybox;
  unsigned skyboxCubemap = gfx::createHdrCubemap("assets/textures/irradiance/");

  // Objects
  std::array<CubePrimitive, 7ull> cubes;
  cubes[0].translate({0, 0, 0});
  // px, nx, py, ny, pz, nz
  cubes[1].translate({+3, 0, 0});
  cubes[2].translate({-3, 0, 0});
  cubes[3].translate({0, +3, 0});
  cubes[4].translate({0, -3, 0});
  cubes[5].translate({0, 0, +3});
  cubes[6].translate({0, 0, -3});

  // Object Colors
  std::array<glm::vec3, 7ull> albedos;
  albedos[0] = glm::vec3(1.f, 1.f, 1.f);
  // red, orange, yellow, green, blue, purple
  albedos[1] = glm::vec3(1.f, 0, 0);
  albedos[2] = glm::vec3(1.f, 0.5f, 0);
  albedos[3] = glm::vec3(1.f, 1.f, 0);
  albedos[4] = glm::vec3(0, 1.f, 0);
  albedos[5] = glm::vec3(0, 0.5f, 1.f);
  albedos[6] = glm::vec3(0.5f, 0, 1);

  // PBL + IBL
  unsigned irradianceMap = gfx::createHdrCubemap("assets/textures/irradiance/");
  unsigned prefilterMap = gfx::createHdrCubemapMipped("assets/textures/prefilter/", 5);
  unsigned brdfLUT = gfx::createHdrTexture("assets/textures/brdf-lut/brdf-lut.hdr");

  // Main graphics loop
  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw models inside the scence
    objectShader.enable();

    // IBL
    objectShader.setInt("irradianceMap", 0);
    objectShader.setInt("prefilterMap", 1);
    objectShader.setInt("brdfLUT", 2);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, brdfLUT);

    // Camera
    objectShader.setVec3("cameraPos", camera.position);

    // Model, View, Projection
    auto model = glm::mat4(1.0f);
    auto view = camera.view();
    auto projection = glm::perspective(glm::radians(70.f), 16.f / 9.f, 0.1f, 100.0f);

    objectShader.setMat4("view", view);
    objectShader.setMat4("projection", projection);

    // Set the properties of the objects
    objectShader.setFloat("metallic", 0.1f);
    objectShader.setFloat("roughness", 0.8f);
    objectShader.setFloat("ao", 0.6f);

    // draw all objects:
    for (int i = 0; i < cubes.size(); i++) {
      objectShader.setVec3("albedo", albedos[i]);
      objectShader.setMat4("model", cubes[i].model);
      cubes[i].draw(&objectShader);
    }

    // change depth function so depth test passes when values are equal to depth buffer's content
    glDepthFunc(GL_LEQUAL);
    skyboxShader.enable();
    // remove translation from the view matrix
    view = glm::mat4(glm::mat3(camera.view()));
    skyboxShader.setMat4("view", view);
    skyboxShader.setMat4("projection", projection);
    skyboxShader.setMat4("skybox", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemap);
    skybox.draw(&skyboxShader);
    // set depth function back to default
    glDepthFunc(GL_LESS);

    glfwSwapBuffers(window);
    glfwPollEvents();

    // handle events
    while (!eventQueue.empty()) {
      const auto event = eventQueue.front();
      switch (event.type) {
        case EventType::kKeyboardEvent: {
          const auto data = std::get<KeyboardEvent>(event.contents());
          switch (data.key) {
            case GLFW_KEY_O: {
              // open a file and import
              // ...
              break;
            }
            case GLFW_KEY_ESCAPE: {
              // close the program
              glfwSetWindowShouldClose(window, true);
              break;
            }
            default: {
              // forward to the camera
              camera.handleKeyboardEvent(data);
              break;
            }
          }
          break;
        }
        case EventType::kMouseButtonEvent: {
          const auto data = std::get<MouseButtonEvent>(event.contents());
          // forward to the camera
          camera.handleMouseButtonEvent(data);
          break;
        }
        case EventType::kMousePositionEvent: {
          const auto data = std::get<MousePositionEvent>(event.contents());
          // forward to the camera
          camera.handlePositionEvent(data);
          break;
        }
        case EventType::kScrollEvent: {
          const auto data = std::get<ScrollEvent>(event.contents());
          // forward to the camera
          camera.handleScrollEvent(data);
          break;
        }
      }
      eventQueue.pop();
    }
  }

  return 0;
}