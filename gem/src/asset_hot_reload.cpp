#include "gem/asset_hot_reload.h"
#include "spdlog/spdlog.h"
#include "gem/engine.h"
#include "gem/utils.h"

namespace gem
{

void reload_shader(asset_t<gl_shader, asset_type::shader> *shader_asset,
                   std::string shader_source) {

}

void clean_delimiters(std::string& input)
{
  for(auto& c : input)
  {
    if(c == '\\')
    {
      c = '/';
    }
  }
}

void GemFileListener::handleFileAction(efsw::WatchID watchid,
                                       const std::string &dir,
                                       const std::string &filename,
                                       efsw::Action action,
                                       std::string oldFilename) {
  switch ( action ) {
  case efsw::Actions::Add: {
    break;
  }
  case efsw::Actions::Delete: {
    break;
  }
  case efsw::Actions::Modified: {
    std::string full_path = dir + filename;
    clean_delimiters(full_path);
    asset_handle ah = asset_handle(full_path, asset_type::shader);
    if (engine::assets.get_asset_load_progress(ah) ==
        asset_load_progress::loaded) {
      spdlog::info("GemFileListener : Reloading : {}{}", dir, filename);
      std::string shader_source = utils::load_string_from_path(full_path);
      auto* shader_asset = engine::assets.get_asset<gl_shader, asset_type::shader>(ah);

      engine::debug_callbacks.add([shader_asset, shader_source]()
      {
          shader_asset->m_data.release();
          shader_asset->m_data = gl_shader::create_from_composite(shader_source);
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
}