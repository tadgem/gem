#pragma once
#include "asset_manager.h";
#include "backend.h"
#include "ecs_system.h"
#include "events.h"
#include "gem/gl/gl_renderer.h"
#include "project.h"
#include "scene.h"

struct DebugCallbackCollection {
  std::vector<std::function<void()>> callbacks;

  void Add(std::function<void()> debug_callback);
};

namespace gem {

class Engine {
public:
  inline static AssetManager assets;
  inline static EventHandler events;
  inline static GLRenderer renderer;
  inline static SceneManager scenes;
  inline static SystemManager systems;
  inline static Project active_project;
  inline static DebugCallbackCollection debug_callbacks;

  static void Init(const glm::ivec2& resolution);
  static void Update();
  static void SaveProjectToDisk(const std::string &filename,
                                   const std::string &directory);
  static void LoadProjectFromDisk(const std::string &filepath);
  static void Shutdown();
};

} // namespace gem