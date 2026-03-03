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
  void addLineSegments(std::vector<glm::vec2>& points, glm::vec4 color, unsigned width);

  void addPoint(glm::vec2 center, glm::vec4 color, unsigned size);

  void addPolygon(std::vector<glm::vec2>& points, glm::vec4 color);
  void addPolygonOutline(std::vector<glm::vec2>& points, glm::vec4 color, unsigned width);

  void addTriangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec4 color);
  void addTriangleOutline(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec4 color, unsigned width);

  void addRect(glm::vec2 p1, glm::vec2 p2, glm::vec4 color);
  void addRectOutline(glm::vec2 p1, glm::vec2 p2, glm::vec4 color, unsigned width);

  void addCircle(glm::vec2 center, glm::vec4 color);
  void addCircleOutline(glm::vec2 center, glm::vec4 color, unsigned width);

  void addSector(glm::vec2 center, float t_start, float t_end, glm::vec4 color);
  void addSectorOutline(glm::vec2 center, float t_start, float t_end, glm::vec4 color, unsigned width);

  void addArc(glm::vec2 center, float t_start, float t_end, glm::vec4 color, unsigned width);

  void addTexturedRect(glm::vec2 p1, glm::vec2 p2, glm::vec4 color, std::shared_ptr<class TextureAtlas> atlas,
                       unsigned subTextureId);

 private:
  // a lot of these commands deal with proceduarally generating
  // arrays of points for triangles or lines. below are a set of 
  // common utilities to prevent code duplication
  
    
 protected:
  struct DrawCommand {
    DrawCommand() = default;
    
    std::size_t begin = 0ull;
    std::size_t end = 0ull;
    int primitive = GL_NONE;
    union {
      unsigned lineWidth;
      unsigned pointSize;
    };
  };

  std::vector<DrawCommand> drawCommands;
  
  void beginTriangles();
  void beginLines(unsigned width);
  void beginPoints(unsigned width);
  void endPrimitive();
};
}  // namespace sb