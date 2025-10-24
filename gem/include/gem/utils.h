#pragma once

#include "gem/AABB.h"
#include "gem/alias.h"
#include "gem/shape.h"
#include "glm.hpp"
#include "spdlog/spdlog.h"
#include <array>
#include <future>
#include <string>

namespace gem {

class Utils {
public:
  static std::string LoadStringFromPath(const std::string &path);
  static void SaveStringToPath(const std::string &path,
                                  const std::string &str);
  static std::vector<u8> LoadBinaryFromPath(const std::string &path);
  static void SaveBinaryToPath(const std::string &path,
                                  std::vector<u8> &bytes);
  static glm::quat GetQuatFromEuler(glm::vec3 euler);
  static glm::mat4 GetModelMatrix(glm::vec3 position, glm::vec3 euler,
                                    glm::vec3 scale);
  static glm::mat3 GetNormalMatrix(glm::mat4 model);
  static glm::vec3 CartesianToSpherical(glm::vec3 normal);
  static glm::vec3 SphericalToCartesian(glm::vec3 spherical);
  static glm::vec3 GetForwardFromEuler(glm::vec3 euler);
  static glm::vec3 GetRightFromEuler(glm::vec3 euler);
  static glm::vec3 GetUpFromEuler(glm::vec3 euler);
  static glm::vec3 GetForwardFromQuat(glm::quat rot);
  static glm::vec3 GetRightFromQuat(glm::quat rot);
  static glm::vec3 GetUpFromQuat(glm::quat rot);
  static glm::vec3 GetMouseWorldPos(glm::vec2 mouse_pos,
                                       glm::vec2 resolution, glm::mat4 &proj,
                                       glm::mat4 &view);
  static float RoundUp(float value, int decimal_places);
  static AABB TransformAABB(const AABB &in, glm::mat4 &model);
  static void ValidateEulerAngles(glm::vec3 &input);
  static glm::vec3 ScreenToWorldRay(glm::vec3 eye_pos, glm::vec2 mouse_pos,
                                       glm::vec2 screen_dim,
                                       glm::mat4 inverse_view,
                                       glm::mat4 inverse_proj);
  static glm::vec3 ScreenToWorldPos(glm::vec2 mouse_pos,
                                       glm::vec2 screen_dim,
                                       glm::mat4 inverse_view,
                                       glm::mat4 inverse_proj);
  static std::array<glm::vec4, 6>
  GetPlanesFromViewProjectionMatrix(glm::mat4 view_proj);

  static std::string GetDirFromPath(const std::string &path);

  // template utils
  template <typename _Ty>
  static bool IsFutureReady(std::future<_Ty> const &o) {
    return o.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
  }
};
} // namespace gem

template <> struct fmt::formatter<glm::vec2> : fmt::formatter<std::string> {
  auto format(glm::vec2 my, format_context &ctx) const -> decltype(ctx.out()) {
    return format_to(ctx.out(), "[{},{}]", my.x, my.y);
  }
};

template <> struct fmt::formatter<glm::vec3> : fmt::formatter<std::string> {
  auto format(glm::vec3 my, format_context &ctx) const -> decltype(ctx.out()) {
    return format_to(ctx.out(), "[{},{},{}]", my.x, my.y, my.z);
  }
};

template <> struct fmt::formatter<glm::vec4> : fmt::formatter<std::string> {
  auto format(glm::vec4 my, format_context &ctx) const -> decltype(ctx.out()) {
    return format_to(ctx.out(), "[{},{},{},{}]", my.x, my.y, my.z, my.w);
  }
};

template <> struct fmt::formatter<glm::mat3> : fmt::formatter<std::string> {
  auto format(glm::mat3 my, format_context &ctx) const -> decltype(ctx.out()) {
    return format_to(ctx.out(), "[{},{},{}]", my[0], my[1], my[2]);
  }
};

template <> struct fmt::formatter<glm::mat4> : fmt::formatter<std::string> {
  auto format(glm::mat4 my, format_context &ctx) const -> decltype(ctx.out()) {
    return format_to(ctx.out(), "[{},{},{},{}]", my[0], my[1], my[2], my[3]);
  }
};
