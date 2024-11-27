#pragma once
#include "asset.h"
#include "efsw/efsw.hpp"

class shader;

namespace gem
{

  class GemFileListener : public efsw::FileWatchListener {
  public:
    void handleFileAction(efsw::WatchID watchid, const std::string &dir,
                          const std::string &filename, efsw::Action action,
                          std::string oldFilename) override;

    efsw::WatchID m_watch_id;
  };


  void reload_shader( asset_t<shader, asset_type::shader>* shader_asset,
                      std::string shader_source);

}