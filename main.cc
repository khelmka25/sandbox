#include <argh.h>

#include "Application/Window.h"
#include "Camera/Camera.h"
#include "Graphics/Texture/Texture.h"
#include "Object/CubePrimitive.h"
#include "Object/Model/Model.h"
#include "Preprocessor/BrdfLut.h"
#include "Preprocessor/Conversion.h"
#include "Preprocessor/Irradiance.h"
#include "Preprocessor/Prefilter.h"

using namespace std::string_view_literals;

int main(int argc, char** argv) {
  // Create the window
  const auto title = "OpenGL Window"sv;
  const auto displayWidth = 640u;
  const auto displayHeight = 360u;
  GLFWwindow* window = gfx::createGLFWwindow(title, displayWidth, displayHeight);

  // PBL + IBL
  unsigned envCubemap, irradianceMap, prefilterMap, brdfLUT;
  // Parse the command line, generate new textures if req
  // "-g/--gen <filename.hdr>" generates a new set of hdr maps
  // "-c/--cache" uses the cached generated textures
  argh::parser cmdl({"-f", "--file"});
  cmdl.parse(argc, argv);
  if (cmdl[{"-g", "--generate"}]) {
    std::string filename;
    cmdl({"f", "file"}) >> filename;
    std::cout << filename << std::endl;
    // generate new maps
    auto dir = std::filesystem::path("assets/textures/input");
    auto filepath = dir / filename;
    std::cout << "Generating new maps using: " << filepath << std::endl;

    // begin generation
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    // convert the given hdr into a cubemap for use
    envCubemap = gfx::convertEnvironmentMap(filepath);
    gfx::exportEnvironmentMap(envCubemap, "assets/textures/input/");
    // irradiance map: 128x128
    irradianceMap = gfx::createIrradianceMap(envCubemap, 128);
    gfx::exportIrradianceCubeMap(irradianceMap, 128, "assets/textures/irradianceMap/");
    // prefilter map: 128x128, 5 mips
    prefilterMap = gfx::createPrefilterMap(envCubemap, 128, 5);
    gfx::exportPrefilterMap(prefilterMap, 128, 5, "assets/textures/prefilterMap/");
    // BRDF-LUT: 512x512
    brdfLUT = gfx::createBrdfLutTexture(512);
    gfx::exportBrdfLutTexture(brdfLUT, 512, "assets/textures/brdfLut/brdfLut.hdr");
    // end generation
    glEnable(GL_DEPTH_TEST);

    // reset the viewport to the desired resolution
    glViewport(0, 0, displayWidth, displayHeight);
  } else if (cmdl[{"-c", "--cache"}]) {
    std::cout << "Using previously generated maps" << std::endl;
    envCubemap = gfx::createHdrCubemap("assets/textures/input/");
    irradianceMap = gfx::createHdrCubemap("assets/textures/irradianceMap/");
    prefilterMap = gfx::createHdrCubemapMipped("assets/textures/prefilterMap/", 5);
    brdfLUT = gfx::createHdrTexture("assets/textures/brdfLut/brdfLut.hdr");
  } else {
    std::cout << "Usage:" << std::endl;
    std::cout << argv[0] << "--gen photo_studio_01_1k.hdr" << std::endl;
    std::cout << argv[0] << "--cache" << std::endl;
    return -1;
  }

  // Create the camera
  Camera camera("cam"sv, {0, 0, 0}, {0, 1, 0}, {1, 0, 0}, 3.f, 0.1f, 0.f, 0.f);

  // Shaders: cubemap background shader
  Shader backgroundShader("assets/shaders/background.vs", "assets/shaders/background.fs");
  // Shaders: pbr + ibl shader
  Shader objectShader("assets/shaders/pbr.vs", "assets/shaders/pbr.fs");

  // Object: background
  CubePrimitive background;

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

  // create position for the camera to orbit around
  glPointSize(3.f);
  unsigned orbitVAO, orbitVBO;
  glGenVertexArrays(1, &orbitVAO);
  glGenBuffers(1, &orbitVBO);
  glBindVertexArray(orbitVAO);
  glm::vec3 orbit{};
  glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(orbit), glm::value_ptr(orbit), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glBindVertexArray(0);
  
  Shader orbitShader("assets/shaders/orbit.vs", "assets/shaders/orbit.fs");

  // Main graphics loop
  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw models inside the scence
    objectShader.use();

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
    objectShader.setVec3("camPos", camera.position());

    // Model, View, Projection
    auto model = glm::mat4(1.0f);
    const auto& view = camera.view();
    // use orthographic projection?
    auto projection = glm::perspective(glm::radians(70.f), 16.f / 9.f, 0.1f, 100.0f);
    // auto projection = glm::ortho(-camera.radius, camera.radius, -camera.radius, camera.radius, 0.f, 100.f);

    objectShader.setMat4("view", view);
    objectShader.setMat4("projection", projection);

    // Set the properties of the objects
    objectShader.setFloat("metallic", 0.1f);
    objectShader.setFloat("roughness", 0.3f);
    objectShader.setFloat("ao", 0.6f);

    // draw all objects:
    for (int i = 0; i < cubes.size(); i++) {
      objectShader.setVec3("albedo", albedos[i]);
      objectShader.setMat4("model", cubes[i].model);
      objectShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(cubes[i].model))));
      cubes[i].draw(&objectShader);
    }

    /* Draw the orbit position of the camera */
    glDepthFunc(GL_ALWAYS);
    glBindVertexArray(orbitVAO);
    // draw the orbit position
    orbitShader.use();
    orbitShader.setVec3("albedo", glm::vec3(0.25, 1, 0.25));
    orbitShader.setMat4("model", camera.orbit());
    orbitShader.setMat4("view", camera.view());
    orbitShader.setMat4("projection", projection);
    glDrawArrays(GL_POINTS, 0, 1);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    
    /* Draw the background */
    // change depth function so depth test passes when values are equal to depth buffer's content
    glDepthFunc(GL_LEQUAL);
    backgroundShader.use();
    // remove translation from the view matrix
    backgroundShader.setMat4("view", glm::mat4(glm::mat3(view)));
    backgroundShader.setMat4("projection", projection);
    backgroundShader.setMat4("environmentMap", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    background.draw(&backgroundShader);
    // set depth function back to default
    glDepthFunc(GL_LESS);

    /* Flush results to screen and poll events */
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