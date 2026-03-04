#include <argh.h>

#include <utility>

#include "Application/Window.h"
#include "Camera/Camera.h"
#include "Data.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/Texture.h"
#include "Object/CubePrimitive.h"
#include "Object/Model/Model.h"
#include "Object/PlanePrimitive.h"
#include "Object/Widget/Axes.h"
#include "Object/Widget/Grid.h"
#include "Object/Widget/Orbit.h"
#include "Preprocessor/BrdfLut.h"
#include "Preprocessor/Environment.h"
#include "Preprocessor/Irradiance.h"
#include "Preprocessor/Prefilter.h"
#include "Graphics/DrawList.h"
#include "Graphics/TextureAtlas.h"
#include "Graphics/CharacterMetric.h"
#include "Graphics/CharacterMetricSet.h"

using namespace std::string_view_literals;

int main(int argc, char** argv) {
  // Create the window
  const auto title = "OpenGL Window"sv;
  sb::data::displayWidth = 640.f;
  sb::data::displayHeight = 360.f;
  GLFWwindow* window = sb::gfx::createGLFWwindow(title, sb::data::displayWidth, sb::data::displayHeight);

  // PBL + IBL
  unsigned envMap3D, irradianceMap, prefilterMap, brdfLUT;
  
  // Parse the command line, generate new textures if req
  // "-g/--gen <filename.hdr>" generates a new set of hdr maps
  // "-c/--cache" uses the cached generated textures
  argh::parser cmdl({"--file", "-F"});
  cmdl.parse(argc, argv);
  if (cmdl[{"-G", "--generate"}]) {
    std::string filename;
    cmdl({"file", "F"}) >> filename;
    if (filename.empty()) {
      std::cout << "Please supply a valid filename" << std::endl;
      return -1;
    }

    std::cout << filename << std::endl;
    // generate new maps
    auto dir = std::filesystem::path("assets/textures/input");
    auto filepath = dir / filename;
    std::cout << "Generating new maps using: " << filepath << std::endl;

    // begin generation
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
  
    // convert the given hdr into a cubemap for use
    envMap3D = sb::EnvironmentMap::create(filepath);

    // irradiance map: 128x128
    irradianceMap = sb::IrradianceMap::create(envMap3D, 128);
    
    // prefilter map: 128x128, 5 mips
    prefilterMap = sb::PrefilterMap::create(envMap3D, 128u, 5u);

    // BRDF-LUT: 512x512
    brdfLUT = sb::BrdfLut::create(512u);
    
    // end generation
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    if (cmdl[{"--export", "-E"}]) {
      sb::EnvironmentMap::saveToFile(envMap3D, "assets/textures/input/");
      sb::IrradianceMap::saveToDir(irradianceMap, 128u, "assets/textures/irradianceMap/");
      sb::PrefilterMap::saveToFile(prefilterMap, 128u, 5u, "assets/textures/prefilterMap/");
      sb::BrdfLut::saveToFile(brdfLUT, 512u, "assets/textures/brdfLut/brdfLut.hdr");
    }

    // reset the viewport to the desired resolution
    glViewport(0, 0, sb::data::displayWidth, sb::data::displayHeight);
  } else if (cmdl[{"--cache", "-C"}]) {
    std::cout << "Using previously generated maps" << std::endl;
    envMap3D = sb::gfx::createHdrCubemap("assets/textures/input/");
    irradianceMap = sb::gfx::createHdrCubemap("assets/textures/irradianceMap/");
    prefilterMap = sb::gfx::createHdrCubemapMipped("assets/textures/prefilterMap/", 5);
    brdfLUT = sb::gfx::createHdrTexture("assets/textures/brdfLut/brdfLut.hdr");
  } else {
    std::cout << "Usage:" << std::endl;
    std::cout << argv[0] << " --generate --file photo_studio_01_1k.hdr" << std::endl;
    std::cout << argv[0] << " --cache" << std::endl;
    return EXIT_FAILURE;
  }

  assert((envMap3D != 0) && "Envionment Map must be initialized");
  assert((irradianceMap != 0) && "Irradiance Map must be initialized");
  assert((prefilterMap != 0) && "Prefilter Map must be initialized");
  assert((brdfLUT != 0) && "BrdfLut must be initialized");

  auto skyboxCubemap = envMap3D;
  
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
  sb::Shader widgetShader("widget"sv, "assets/shaders/widget/widget.vs", "assets/shaders/widget/widget.fs");
  sb::Shader selectionShader("selection"sv, "assets/shaders/utility/selection.vs", "assets/shaders/utility/selection.fs");
  sb::Shader backgroundShader("background"sv, "assets/shaders/background.vs", "assets/shaders/background.fs");
  sb::Shader outlineShader("outline"sv, "assets/shaders/pbr.vs", "assets/shaders/utility/outline.fs");
  sb::Shader uiShader("ui"sv, "assets/shaders/widget/ui.vs", "assets/shaders/widget/ui.fs");
  
  // Shaders: pbr + ibl shader
  sb::Shader pbrShader("pbr"sv, "assets/shaders/pbr.vs", "assets/shaders/pbr.fs");

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

  // 0 = no selection, 1 = 0th object selected
  int sceneSelection = 0;
  
  auto atlas = std::make_shared<sb::TextureAtlas>();
  // utility textures
  atlas->addBlankTexture();
  atlas->addMissingTexture();
  // various fonts, requierd to save the output base name ids for later use
  const auto monaspaceNV16 = atlas->addFont("assets/fonts/Monaspace Neon Var.ttf", 16);
  const auto monaspaceNV24 = atlas->addFont("assets/fonts/Monaspace Neon Var.ttf", 24);
  const auto monaspaceNV32 = atlas->addFont("assets/fonts/Monaspace Neon Var.ttf", 32);
  // fun textures for testing
  const auto nasaTextureId = atlas->addTexture("assets/textures/nasa-meatball.png");

  // recompute the arrangement of the texture atlas:
  atlas->recompute();
  // upload the subtexture data to the GL
  atlas->rebuffer();

  // Character Metric Set
  const auto monaspaceMetricSet16 = std::make_shared<sb::CharacterMetricSet>("assets/fonts/Monaspace Neon Var.ttf", 16);
  const auto monaspaceMetricSet24 = std::make_shared<sb::CharacterMetricSet>("assets/fonts/Monaspace Neon Var.ttf", 24);
  const auto monaspaceMetricSet32 = std::make_shared<sb::CharacterMetricSet>("assets/fonts/Monaspace Neon Var.ttf", 32);

  // create drawlist:
  auto drawlist = std::make_shared<sb::DrawList>();
  drawlist->addTexturedRect({100, 100}, {600, 600}, glm::vec4(1), atlas, nasaTextureId);
  drawlist->addText({100, 100}, "Hello, World!"sv, glm::vec4(1.f), atlas, monaspaceNV16, monaspaceMetricSet16);
  drawlist->addText({0, 0}, "Small Text", glm::vec4(1.f), atlas, monaspaceNV16, monaspaceMetricSet16);
  drawlist->addText({0, 16}, "Medium Text", glm::vec4(1.f), atlas, monaspaceNV24, monaspaceMetricSet24);
  drawlist->addText({0, 40}, "Large Text", glm::vec4(1.f), atlas, monaspaceNV32, monaspaceMetricSet32);


  drawlist->rebuffer();

  // Main graphics loop
  while (!glfwWindowShouldClose(window)) {
    // Model, View, Projection
    const auto& view = camera.view();
    auto projection =
        glm::perspective(glm::radians(70.f), sb::data::displayWidth / sb::data::displayHeight, 0.1f, 1000.0f);

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
    // pbrShader.setFloat("metallic", 0.1f);
    pbrShader.setFloat("metallic", std::abs(std::sin(glfwGetTime() / 10.f)));
    // pbrShader.setFloat("roughness", 0.3f);
    pbrShader.setFloat("roughness", std::abs(std::cos(glfwGetTime() / 10.f)));
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

    // {
    //   glCullFace(GL_FRONT);
    //   /* Draw the background */
    //   // change depth function so depth test passes when values are equal to depth buffer's content
    //   glDepthFunc(GL_LEQUAL);
    //   backgroundShader.use();
    //   // remove translation from the view matrix
    //   backgroundShader.setMat4("view", glm::mat4(glm::mat3(view)));
    //   backgroundShader.setMat4("projection", projection);
    //   backgroundShader.setMat4("environmentMap", 0);
    //   glActiveTexture(GL_TEXTURE0);
    //   glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemap);
    //   background.draw(&backgroundShader);
    //   // set depth function back to default
    //   glDepthFunc(GL_LESS);

    //   glCullFace(GL_BACK);
    // }

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
      glDisable(GL_CULL_FACE);
      uiShader.use();
      // source texture 2D
      uiShader.setInt("atlas", 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, atlas->textureName);
      // proj, view, model
      const glm::mat4 uiProj = glm::ortho(0.f, +sb::data::displayWidth, +sb::data::displayHeight, 0.f, -1.0f, 1.0f);
      glm::mat4 uiModel(1.f);
      // uiModel = glm::translate(uiModel, glm::vec3(0, +sb::data::displayHeight, 0.f));
      // update in the shader:
      uiShader.setMat4("projection", uiProj);
      uiShader.setMat4("model", uiModel);

      // draw here:
      drawlist->draw(); 
      glEnable(GL_CULL_FACE);
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

  return EXIT_SUCCESS;
}