#include "Object/PlanePrimitive.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace sb {

// ccw winding order:
//          ^
//      3-- | --2
//      |T1 | / |
// <------- / ------>
//      | / | T0|
//      0-- | --1
//          V
// T0: 0, 1, 2
// T1: 0, 2, 3
glm::vec3 PlanePrimitive::vertices[4] = {
    {-1.0f, -1.0f, 0.0f},  // 0
    {+1.0f, -1.0f, 0.0f},  // 1
    {+1.0f, +1.0f, 0.0f},  // 2
    {-1.0f, +1.0f, 0.0f},  // 3
};

glm::vec3 PlanePrimitive::normals[4] = {
    {0.0f, 0.0f, 1.0f},  // tr
    {0.0f, 0.0f, 1.0f},  // tl
    {0.0f, 0.0f, 1.0f},  // br
    {0.0f, 0.0f, 1.0f},  // bl
};

glm::vec2 PlanePrimitive::uvs[4] = {
    {+0.0f, +0.0f},  // bl
    {+1.0f, +0.0f},  // br
    {+1.0f, +1.0f},  // tr
    {+0.0f, +1.0f},  // tl
};

unsigned PlanePrimitive::indices[6] = {
    0, 1, 2, 0, 2, 3,
};
}  // namespace sb