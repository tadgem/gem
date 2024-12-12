#pragma once
#include "im3d/im3d.h"

namespace gem {

class DebugDraw {
public:

  static void DrawLine(const glm::vec3& a, const glm::vec3& b, float thickness, const Im3d::Color& col);
  static void DrawFrustum(const glm::mat4& vp, float thickness, const Im3d::Color& col);
  static void DrawBoundingBox(const glm::vec3& min, const glm::vec3& max, float thickness, const Im3d::Color& col );
};
}