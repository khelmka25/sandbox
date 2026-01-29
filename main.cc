#include <argh.h>

#include "Application/Window.h"
#include "Camera/Camera.h"
#include "Graphics/Texture/Texture.h"
#include "Object/CubePrimitive.h"
#include "Object/Model/Model.h"
#include "Object/Widget/Axes.h"
#include "Object/Widget/Grid.h"
#include "Object/Widget/Orbit.h"
#include "Preprocessor/BrdfLut.h"
#include "Preprocessor/Conversion.h"
#include "Preprocessor/Irradiance.h"
#include "Preprocessor/Prefilter.h"

using namespace std::string_view_literals;

using namespace sb;

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

  // Widgets
  Shader widgetShader("assets/shaders/widget.vs", "assets/shaders/widget.fs");
  std::unique_ptr<Object> orbitObject = std::make_unique<Orbit>();
  std::unique_ptr<Object> axesObject = std::make_unique<Axes>(glm::vec3(-5, 0, -5));
  std::unique_ptr<Object> gridObject = std::make_unique<Grid>();

  // Object: background
  Shader backgroundShader("assets/shaders/background.vs", "assets/shaders/background.fs");
  CubePrimitive background(glm::vec3(0, 0, 0), glm::vec4(1.f, 1.f, 1.f, 1));

  // Shaders: pbr + ibl shader
  Shader pbrShader("assets/shaders/pbr.vs", "assets/shaders/pbr.fs");
  std::vector<std::unique_ptr<Object>> objects;
  // at origin
  objects.push_back(std::make_unique<CubePrimitive>(glm::vec3(0, 0, 0), glm::vec4(1.f, 1.f, 1.f, 1)));
  // px, nx, py, ny, pz, nz
  objects.push_back(std::make_unique<CubePrimitive>(glm::vec3(+3, 0, 0), glm::vec4(1.f, 0, 0, 1)));
  objects.push_back(std::make_unique<CubePrimitive>(glm::vec3(-3, 0, 0), glm::vec4(1.f, 0.5f, 0, 1)));
  objects.push_back(std::make_unique<CubePrimitive>(glm::vec3(0, +3, 0), glm::vec4(1.f, 1.f, 0, 1)));
  objects.push_back(std::make_unique<CubePrimitive>(glm::vec3(0, -3, 0), glm::vec4(0, 1.f, 0, 1)));
  objects.push_back(std::make_unique<CubePrimitive>(glm::vec3(0, 0, +3), glm::vec4(0, 0.5f, 1.f, 1)));
  objects.push_back(std::make_unique<CubePrimitive>(glm::vec3(0, 0, -3), glm::vec4(0.5f, 0, 1, 1)));

  // models...
  // objects.push_back(std::make_unique<Model>("assets/objects/..."));

  glPointSize(4.f);
  glLineWidth(2.f);
  glEnable(GL_MULTISAMPLE);

  // Main graphics loop
  while (!glfwWindowShouldClose(window)) {
    glClearColor(1.f, 1.f, 1.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw models inside the scence
    pbrShader.use();

    // IBL
    pbrShader.setInt("irradianceMap", 0);
    pbrShader.setInt("prefilterMap", 1);
    pbrShader.setInt("brdfLUT", 2);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, brdfLUT);

    // Camera
    pbrShader.setVec3("camPos", camera.position());

    // Model, View, Projection
    const auto& view = camera.view();
    // use orthographic projection?
    auto projection = glm::perspective(glm::radians(70.f), 16.f / 9.f, 0.1f, 100.0f);
    // auto projection = glm::ortho(-camera.radius, camera.radius, -camera.radius, camera.radius, 0.f, 100.f);

    pbrShader.setMat4("view", view);
    pbrShader.setMat4("projection", projection);

    // Set the properties of the objects
    pbrShader.setFloat("metallic", 0.1f);
    pbrShader.setFloat("roughness", 0.3f);
    pbrShader.setFloat("ao", 0.6f);

    // draw all objects:
    for (auto& object : objects) {
      object->draw(&pbrShader);
    }

    /* Draw the orbit position of the camera */
    glDepthFunc(GL_ALWAYS);

    widgetShader.use();
    glm::mat4 axesModel(1);
    axesModel = glm::translate(axesModel, glm::vec3(-5.1, 0, -5.1));
    widgetShader.setMat4("model", axesModel);
    widgetShader.setMat4("view", camera.view());
    widgetShader.setMat4("projection", projection);
    axesObject->draw(&widgetShader);

    glDepthFunc(GL_LESS);
    widgetShader.use();
    widgetShader.setMat4("model", glm::mat4(1));
    gridObject->draw(&widgetShader);

    glDepthFunc(GL_ALWAYS);

    // draw the orbit position
    widgetShader.use();
    widgetShader.setMat4("model", camera.orbit());
    widgetShader.setMat4("view", camera.view());
    widgetShader.setMat4("projection", projection);
    orbitObject->draw(&widgetShader);
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
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
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