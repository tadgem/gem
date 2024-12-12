#pragma once
#include "asset.h"
#include "efsw/efsw.hpp"

class shader;

namespace gem {

class GemFileWatchListener : public efsw::FileWatchListener {
public:
  void handleFileAction(efsw::WatchID watchid, const std::string &dir,
                        const std::string &filename, efsw::Action action,
                        std::string oldFilename) override;

  efsw::WatchID m_watch_id;
};

void reload_shader(TAsset<shader, AssetType::shader> *shader_asset,
                   std::string shader_source);

} // namespace gem