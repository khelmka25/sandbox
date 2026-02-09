#include <argh.h>

#include <utility>

#include "Application/Window.h"
#include "Camera/Camera.h"
#include "Data.h"
#include "Graphics/Text/Atlas.h"
#include "Object/PlanePrimitive.h"
#include "Graphics/Text/Text.h"
#include "Graphics/Texture/Texture.h"
#include "Graphics/Framebuffer/Framebuffer.h"
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

int main(int argc, char** argv) {
  // Create the window
  const auto title = "OpenGL Window"sv;
  sb::data::displayWidth = 640.f;
  sb::data::displayHeight = 360.f;
  GLFWwindow* window = sb::gfx::createGLFWwindow(title, sb::data::displayWidth, sb::data::displayHeight);

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
    envCubemap = sb::gfx::convertEnvironmentMap(filepath);
    sb::gfx::exportEnvironmentMap(envCubemap, "assets/textures/input/");
    // irradiance map: 128x128
    irradianceMap = sb::gfx::createIrradianceMap(envCubemap, 128);
    sb::gfx::exportIrradianceCubeMap(irradianceMap, 128, "assets/textures/irradianceMap/");
    // prefilter map: 128x128, 5 mips
    prefilterMap = sb::gfx::createPrefilterMap(envCubemap, 128, 5);
    sb::gfx::exportPrefilterMap(prefilterMap, 128, 5, "assets/textures/prefilterMap/");
    // BRDF-LUT: 512x512
    brdfLUT = sb::gfx::createBrdfLutTexture(512);
    sb::gfx::exportBrdfLutTexture(brdfLUT, 512, "assets/textures/brdfLut/brdfLut.hdr");
    // end generation
    glEnable(GL_DEPTH_TEST);

    // reset the viewport to the desired resolution
    glViewport(0, 0, sb::data::displayWidth, sb::data::displayHeight);
  } else if (cmdl[{"-c", "--cache"}]) {
    std::cout << "Using previously generated maps" << std::endl;
    envCubemap = sb::gfx::createHdrCubemap("assets/textures/input/");
    irradianceMap = sb::gfx::createHdrCubemap("assets/textures/irradianceMap/");
    prefilterMap = sb::gfx::createHdrCubemapMipped("assets/textures/prefilterMap/", 5);
    brdfLUT = sb::gfx::createHdrTexture("assets/textures/brdfLut/brdfLut.hdr");
  } else {
    std::cout << "Usage:" << std::endl;
    std::cout << argv[0] << "--gen photo_studio_01_1k.hdr" << std::endl;
    std::cout << argv[0] << "--cache" << std::endl;
    return -1;
  }

  auto skyboxCubemap = envCubemap;
  // blurred background or the hdr cubmap?
  if (cmdl[{"--no-skybox"}]) {
    skyboxCubemap = irradianceMap;
  }

  /* GL State Settings */
  glPointSize(4.f);
  glLineWidth(2.f);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_STENCIL_TEST);
  glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
  // void glStencilOp( 	GLenum sfail,
  // 	GLenum dpfail,
  // 	GLenum dppass);
  glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);

  // Create the camera
  sb::Camera camera("cam"sv, {0, 0, 0}, {0, 1, 0}, {1, 0, 0}, 3.f, 0.1f, 0.f, 0.f);

  // Shaders
  sb::Shader widgetShader("widget"sv, "assets/shaders/widget.vs", "assets/shaders/widget.fs");
  sb::Shader selectionShader("selection"sv, "assets/shaders/selection.vs", "assets/shaders/selection.fs");
  sb::Shader backgroundShader("background"sv, "assets/shaders/background.vs", "assets/shaders/background.fs");
  sb::Shader outlineShader("outline"sv, "assets/shaders/pbr.vs", "assets/shaders/outline.fs");
  sb::Shader textShader("text"sv, "assets/shaders/text.vs", "assets/shaders/text.fs");
  // Shaders: pbr + ibl shader
  sb::Shader pbrShader("pbr"sv, "assets/shaders/pbr.vs", "assets/shaders/pbr.fs");

  // Text atlas
  sb::Atlas atlas;
  sb::createAtlas("assets/fonts/Monaspace Neon Var.ttf", 16, &atlas);

  // Widgets
  std::unique_ptr<sb::Object> orbitObject = std::make_unique<sb::Orbit>();
  std::unique_ptr<sb::Object> axesObject = std::make_unique<sb::Axes>(glm::vec3(-5, 0, -5));
  std::unique_ptr<sb::Object> gridObject = std::make_unique<sb::Grid>();

  // Object: background
  sb::CubePrimitive background(glm::vec3(0, 0, 0), glm::vec4(1.f, 1.f, 1.f, 1));

  std::vector<std::unique_ptr<sb::Object>> objects;
  // at origin
  objects.push_back(std::make_unique<sb::CubePrimitive>(glm::vec3(0, 0, 0), glm::vec4(1.f, 1.f, 1.f, 1)));
  // px, nx, py, ny, pz, nz
  objects.push_back(std::make_unique<sb::CubePrimitive>(glm::vec3(+3, 0, 0), glm::vec4(1.f, 0, 0, 1)));
  objects.push_back(std::make_unique<sb::CubePrimitive>(glm::vec3(-3, 0, 0), glm::vec4(1.f, 0.5f, 0, 1)));
  objects.push_back(std::make_unique<sb::CubePrimitive>(glm::vec3(0, +3, 0), glm::vec4(1.f, 1.f, 0, 1)));
  objects.push_back(std::make_unique<sb::CubePrimitive>(glm::vec3(0, -3, 0), glm::vec4(0, 1.f, 0, 1)));
  objects.push_back(std::make_unique<sb::CubePrimitive>(glm::vec3(0, 0, +3), glm::vec4(0, 0.5f, 1.f, 1)));
  objects.push_back(std::make_unique<sb::CubePrimitive>(glm::vec3(0, 0, -3), glm::vec4(0.5f, 0, 1, 1)));

  // selection utilities
  sb::gfx::Framebuffer framebuffer;
  sb::gfx::createFramebuffer(1920, 1080, &framebuffer);

  sb::PlanePrimitive plane;

  // text prototype
  sb::Text text;
  sb::createText("Example Text", 16, 16, &atlas, &text);

  // 0 = no selection, 1 = 0th object selected
  int sceneSelection = 0;

  // Main graphics loop
  while (!glfwWindowShouldClose(window)) {
    // Model, View, Projection
    const auto& view = camera.view();
    auto projection = glm::perspective(glm::radians(70.f), sb::data::displayWidth / sb::data::displayHeight, 0.1f, 100.0f);

    // bind the selection framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
    // draw the background as 0
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    glViewport(0, 0, sb::data::displayWidth, sb::data::displayHeight);

    // draw each of the objects in a unique color that can be decoded later
    selectionShader.use();
    selectionShader.setMat4("view", view);
    selectionShader.setMat4("projection", projection);
    for (int i = 0; i < objects.size(); i++) {
      auto& object = objects.at(i);
      auto const id = i + 1;
      // color: encoded into rgb output
      glm::vec3 color;
      color.r = float((id >> 0x00) & 0xff) / 255.f;
      color.g = float((id >> 0x08) & 0xff) / 255.f;
      color.b = float((id >> 0x10) & 0xff) / 255.f;
      selectionShader.setVec3("color", color);
      // draw the object
      object->draw(&selectionShader);
    }

    // unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, sb::data::displayWidth, sb::data::displayHeight);
    // draw the entire scene:
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Camera
    pbrShader.use();
    pbrShader.setVec3("camPos", camera.position());

    pbrShader.setInt("irradianceMap", 0);
    pbrShader.setInt("prefilterMap", 1);
    pbrShader.setInt("brdfLUT", 2);

    // IBL
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, brdfLUT);

    pbrShader.setMat4("view", view);
    pbrShader.setMat4("projection", projection);

    // Set the properties of the objects
    pbrShader.setFloat("metallic", 0.1f);
    pbrShader.setFloat("roughness", 0.3f);
    pbrShader.setFloat("ao", 0.6f);

    // draw all the objects in the world, writing into stencil if selected
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0x00);
    for (int i = 0; i < objects.size(); i++) {
      auto& object = objects.at(i);
      if (std::cmp_equal(sceneSelection - 1, i)) {
        // enable writing into stencil buffer
        glStencilMask(0xff);
        // draw the object as normal
        object->draw(&pbrShader);
        // disable writing
        glStencilMask(0x00);
      } else {
        object->draw(&pbrShader);
      }
    }

    plane.draw(&pbrShader);

    glStencilFunc(GL_ALWAYS, 1, 0xff);
    glStencilMask(0x00);

    {
      /* Draw the widgets position of the camera */
      glDepthFunc(GL_ALWAYS);

      widgetShader.use();
      if (sceneSelection > 0) {
        glm::mat4 axesModel(1);
        axesModel = glm::translate(axesModel, objects[sceneSelection - 1]->position());
        widgetShader.setMat4("model", axesModel);
        widgetShader.setMat4("view", camera.view());
        widgetShader.setMat4("projection", projection);
        axesObject->draw(&widgetShader);
      }

      /* Draw the Grid */
      // glDepthFunc(GL_LESS);
      // widgetShader.use();
      // widgetShader.setMat4("model", glm::mat4(1));
      // gridObject->draw(&widgetShader);
      // glDepthFunc(GL_ALWAYS);

      // draw the orbit position
      widgetShader.use();
      widgetShader.setMat4("model", camera.orbit());
      widgetShader.setMat4("view", camera.view());
      widgetShader.setMat4("projection", projection);
      orbitObject->draw(&widgetShader);

      glDepthFunc(GL_LESS);
    }

    {
      /* Draw the background */
      // change depth function so depth test passes when values are equal to depth buffer's content
      glDepthFunc(GL_LEQUAL);
      backgroundShader.use();
      // remove translation from the view matrix
      backgroundShader.setMat4("view", glm::mat4(glm::mat3(view)));
      backgroundShader.setMat4("projection", projection);
      backgroundShader.setMat4("environmentMap", 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemap);
      background.draw(&backgroundShader);
      // set depth function back to default
      glDepthFunc(GL_LESS);
    }

    if (std::cmp_greater(sceneSelection, 0)) {
      /* draw the selected object outline using the stencil */
      glDisable(GL_DEPTH_TEST);
      
      glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
      glStencilMask(0x00);  // disable writing to stencil buffer

      auto& object = objects.at(sceneSelection - 1);

      // draw an outline first
      glLineWidth(2.f);
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      outlineShader.use();
      outlineShader.setMat4("view", view);
      outlineShader.setMat4("projection", projection);
      object->draw(&outlineShader);
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      glStencilMask(0xFF);
      glStencilFunc(GL_ALWAYS, 1, 0xff);

      glEnable(GL_DEPTH_TEST);
    }

    {
      // Draw the text into the scene
      textShader.use();
      textShader.setInt("atlas", 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, atlas.texture);
      glm::mat4 projection2 = glm::ortho(0.f, sb::data::displayWidth, 0.f, sb::data::displayHeight);
      textShader.setMat4("projection", projection2);
      // textShader.setMat4("view", view);
      // glm::mat4 txtModel(1);
      // txtModel = glm::scale(txtModel, glm::vec3(1.f / 96.f, 1.f / 96.f, 1));
      // textShader.setMat4("model", txtModel);
      glBindVertexArray(text.vao);
      glDrawArrays(GL_TRIANGLES, 0, 3 * text.triCount);
      glBindVertexArray(0);
    }

    /* Flush results to screen and poll events */
    glfwSwapBuffers(window);
    glfwPollEvents();

    // handle events
    glm::vec2 lastMousePos;
    while (!sb::eventQueue.empty()) {
      const auto event = sb::eventQueue.front();
      switch (event.type) {
        case sb::EventType::kKeyboardEvent: {
          const auto data = std::get<sb::KeyboardEvent>(event.contents());
          switch (data.key) {
            case GLFW_KEY_O: {
              // open a file and import
              // ...
              break;
            }
            case GLFW_KEY_B: {
              if (data.action == GLFW_PRESS) {
                static bool flag{1};
                flag ^= 1;
                if (flag) {
                  glDisable(GL_BLEND);
                } else {
                  glEnable(GL_BLEND);
                }
              }
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
        case sb::EventType::kMouseButtonEvent: {
          const auto data = std::get<sb::MouseButtonEvent>(event.contents());
          if ((data.button == GLFW_MOUSE_BUTTON_LEFT) && (data.action == GLFW_PRESS)) {
            // sample the pixel at the last mouse position
            int xpos = lastMousePos.x;
            int ypos = lastMousePos.y;

            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
            // void glReadPixels( 	GLint x,
            //   GLint y,
            //   GLsizei width,
            //   GLsizei height,
            //   GLenum format,
            //   GLenum type,
            //   void * data);
            std::uint8_t pixel[3];
            glReadPixels(xpos, sb::data::displayHeight - ypos, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &pixel);
            int r = pixel[0];
            int g = pixel[1];
            int b = pixel[2];
            sceneSelection = (r << 0x00) | (g << 0x08) | (b << 0x10);
            // std::cout << sceneSelection << std::endl;

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

          } else {
            // forward to the camera
            camera.handleMouseButtonEvent(data);
          }
          break;
        }
        case sb::EventType::kMousePositionEvent: {
          const auto data = std::get<sb::MousePositionEvent>(event.contents());
          lastMousePos.x = data.xpos;
          lastMousePos.y = data.ypos;
          // forward to the camera
          camera.handlePositionEvent(data);
          break;
        }
        case sb::EventType::kScrollEvent: {
          const auto data = std::get<sb::ScrollEvent>(event.contents());
          // forward to the camera
          camera.handleScrollEvent(data);
          break;
        }
      }
      sb::eventQueue.pop();
    }
  }

  return 0;
}