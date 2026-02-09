#include "Object/PlanePrimitive.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace sb {
// clockwise winding order
// tr---tl
// |t0/t1|
// br---bl
// tri0: tr, tl, br
// tri1: tl, bl, br
glm::vec3 PlanePrimitive::vertices[4] = {
    {+1.0f, +1.0f, 0.0f},  // tr
    {-1.0f, +1.0f, 0.0f},  // tl
    {+1.0f, -1.0f, 0.0f},  // br
    {-1.0f, -1.0f, 0.0f},  // bl
};

glm::vec3 PlanePrimitive::normals[4] = {
    {0.0f, 0.0f, 1.0f},  // tr
    {0.0f, 0.0f, 1.0f},  // tl
    {0.0f, 0.0f, 1.0f},  // br
    {0.0f, 0.0f, 1.0f},  // bl
};

glm::vec2 PlanePrimitive::uvs[4] = {
    {1.0f, 1.0f},  // tr
    {0.0f, 1.0f},  // tl
    {1.0f, 0.0f},  // br
    {0.0f, 0.0f},  // bl
};

unsigned PlanePrimitive::indices[6] = {
    0, 1, 2, 1, 3, 2,
};
}  // namespace sb