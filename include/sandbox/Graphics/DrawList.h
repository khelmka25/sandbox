#pragma once

#include <glad/glad.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <iostream>
#include <memory>
#include <vector>

#include "Graphics/GeometryList.h"

namespace sb {
struct DrawList : public GeometryList {
 public:
  DrawList();

  using GeometryList::empty;
  void clear();
  using GeometryList::rebuffer;

  void draw();

 public:
  void addText(glm::vec2 p1, std::string_view text, glm::vec4 color, std::shared_ptr<class TextureAtlas> atlas, std::shared_ptr<class CharacterMetricSet> metricSet);

  void addEllipse(glm::vec2 p1, glm::vec2 p2, glm::vec4 color);
  void addEllipseOutline(glm::vec2 p1, glm::vec2 p2, glm::vec4 color, unsigned width);

  void addLineSegment(glm::vec2 p1, glm::vec2 p2, glm::vec4 color, unsigned width);
  void addLineSegments(std::vector<glm::vec2>, glm::vec4 color, unsigned width);

  void addPoint(glm::vec2 center, glm::vec4 color);

  void addPolygon(std::vector<glm::vec2> points, glm::vec4 color);
  void addPolygonOutline(std::vector<glm::vec2> points, glm::vec4 color, unsigned width);

  void addTriangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec4 color);

  void addRect(glm::vec2 p1, glm::vec2 p2, glm::vec4 color);
  void addRectOutline(glm::vec2 p1, glm::vec2 p2, glm::vec4 color, unsigned width);

  void addCircle(glm::vec2 center, glm::vec4 color);
  void addCircleOutline(glm::vec2 center, glm::vec4 color, unsigned width);

  void addPieSlice(glm::vec2 center, glm::vec4 color);
  void addPieSliceOutline(glm::vec2 center, glm::vec4 color, unsigned width);

  void addTexturedRect(glm::vec2 p1, glm::vec2 p2, glm::vec4 color, std::shared_ptr<class TextureAtlas> atlas,
                       unsigned subTextureId);

 protected:
  struct DrawCommand {
    std::size_t begin;
    std::size_t end;
    int primitive;
  };

  std::vector<DrawCommand> drawCommands;
  
  void beginPrimitive(int primitive);
  void endPrimitive();
};
}  // namespace sb