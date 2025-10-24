#include "gem/asset_hot_reload.h"
#include "gem/engine.h"
#include "gem/utils.h"
#include "spdlog/spdlog.h"

namespace gem {

void ReloadShader(TAsset<GLShader, AssetType::shader> *shader_asset,
                   std::string shader_source) {}

void clean_delimiters(std::string &input) {
  for (auto &c : input) {
    if (c == '\\') {
      c = '/';
    }
  }
}

void GemFileWatchListener::handleFileAction(efsw::WatchID watchid,
                                         const std::string &dir,
                                         const std::string &filename,
                                         efsw::Action action,
                                         std::string oldFilename) {
  switch (action) {
  case efsw::Actions::Add: {
    break;
  }
  case efsw::Actions::Delete: {
    break;
  }
  case efsw::Actions::Modified: {
    std::string full_path = dir + filename;
    clean_delimiters(full_path);
    AssetHandle ah = AssetHandle(full_path, AssetType::shader);
    if (Engine::assets.GetLoadProgress(ah) ==
        AssetLoadProgress::loaded) {
      spdlog::info("GemFileListener : Reloading : {}{}", dir, filename);
      std::string shader_source = Utils::LoadStringFromPath(full_path);
      auto *shader_asset =
          Engine::assets.GetAsset<GLShader, AssetType::shader>(ah);

      Engine::debug_callbacks.Add([shader_asset, shader_source]() {
        shader_asset->m_data.Release();
        shader_asset->m_data = GLShader(shader_source);
      });
    }
    break;
  }
  case efsw::Actions::Moved: {
    spdlog::info("GemFileListener : Moved : Old : {}, New : {}{}", oldFilename,
                 dir, filename);
    break;
  }
  default:
    spdlog::info("GemFileListener : Unknown File Action :(");
  }
}
} // namespace gem