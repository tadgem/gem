#include "gem/im3d_gem.h"
void gem::DebugDraw::DrawLine(const glm::vec3 &a, const glm::vec3 &b,
                         float thickness, const Im3d::Color &col) {
  Im3d::Context& ctx = Im3d::GetContext();
  ctx.begin(Im3d::PrimitiveMode_Lines);
  ctx.vertex(&a[0], thickness, col);
  ctx.vertex(&b[0], thickness, col);
  ctx.end();
}
void gem::DebugDraw::DrawFrustum(const glm::mat4& vp, float thickness, const Im3d::Color& col) {
  const glm::mat4 inv = glm::inverse(vp);

  constexpr glm::vec4 f[8u] =
  {
      // near face
      {1, 1, -1, 1.f},
      {-1, 1, -1, 1.f},
      {1, -1, -1, 1.f},
      {-1, -1, -1, 1.f},

      // far face
      {1, 1, 1, 1.f},
      {-1, 1, 1 , 1.f},
      {1, -1, 1 , 1.f},
      {-1, -1,1, 1.f},
  };

  glm::vec3 v[8u];
  for (int i = 0; i < 8; i++)
  {
    glm::vec4 ff = inv * f[i];
    v[i].x = ff.x / ff.w;
    v[i].y = ff.y / ff.w;
    v[i].z = ff.z / ff.w;
  }

  DrawLine(v[0], v[1], 2.0f,  col);
  DrawLine(v[0], v[2], 2.0f,  col);
  DrawLine(v[3], v[1], 2.0f,  col);
  DrawLine(v[3], v[2], 2.0f,  col);

  DrawLine(v[4], v[5], 2.0f,  col);
  DrawLine(v[4], v[6], 2.0f,  col);
  DrawLine(v[7], v[5], 2.0f,  col);
  DrawLine(v[7], v[6], 2.0f,  col);

  DrawLine(v[0], v[4], 2.0f,  col);
  DrawLine(v[1], v[5], 2.0f,  col);
  DrawLine(v[3], v[7], 2.0f,  col);
  DrawLine(v[2], v[6], 2.0f,  col);

}
void gem::DebugDraw::DrawBoundingBox(const glm::vec3 &min, const glm::vec3 &max,
                                     float thickness, const Im3d::Color &col) {
  Im3d::Context& ctx = Im3d::GetContext();
#if IM3D_CULL_PRIMITIVES
  if (!ctx.isVisible(_min, _max))
  {
    return;
  }
#endif
  ctx.begin(Im3d::PrimitiveMode_LineLoop);
  ctx.vertex(Im3d::Vec3(min.x, min.y, min.z), thickness, col);
  ctx.vertex(Im3d::Vec3(max.x, min.y, min.z), thickness, col);
  ctx.vertex(Im3d::Vec3(max.x, min.y, max.z), thickness, col);
  ctx.vertex(Im3d::Vec3(min.x, min.y, max.z), thickness, col);
  ctx.end();
  ctx.begin(Im3d::PrimitiveMode_LineLoop);
  ctx.vertex(Im3d::Vec3(min.x, max.y, min.z), thickness, col);
  ctx.vertex(Im3d::Vec3(max.x, max.y, min.z), thickness, col);
  ctx.vertex(Im3d::Vec3(max.x, max.y, max.z), thickness, col);
  ctx.vertex(Im3d::Vec3(min.x, max.y, max.z), thickness, col);
  ctx.end();
  ctx.begin(Im3d::PrimitiveMode_Lines);
  ctx.vertex(Im3d::Vec3(min.x, min.y, min.z), thickness, col);
  ctx.vertex(Im3d::Vec3(min.x, max.y, min.z), thickness, col);
  ctx.vertex(Im3d::Vec3(max.x, min.y, min.z), thickness, col);
  ctx.vertex(Im3d::Vec3(max.x, max.y, min.z), thickness, col);
  ctx.vertex(Im3d::Vec3(min.x, min.y, max.z), thickness, col);
  ctx.vertex(Im3d::Vec3(min.x, max.y, max.z), thickness, col);
  ctx.vertex(Im3d::Vec3(max.x, min.y, max.z), thickness, col);
  ctx.vertex(Im3d::Vec3(max.x, max.y, max.z), thickness, col);
  ctx.end();
}
