#pragma once
#include <functional>
#include <future>
#include <map>
#include <memory>
#include "gem/asset.h"
#include "gem/asset_hot_reload.h"

namespace gem {

using asset_load_callback   = void (*)(asset_intermediate *);
using asset_loaded_callback = std::function<void(asset *)>;
using asset_unload_callback = void (*)(asset *);

enum class asset_load_progress { not_loaded, loading, loaded, unloading };

struct asset_load_info {
  std::string m_path;
  asset_type m_type;

  bool operator==(const asset_load_info &o) const {
    return m_path == o.m_path && m_type == o.m_type;
  }

  bool operator<(const asset_load_info &o) const {
    return m_path.size() < o.m_path.size();
  }

  asset_handle to_handle();
};

struct asset_load_return {
  asset_intermediate *m_loaded_asset_intermediate = nullptr;
  // additional assets that may be required to completely load this asset
  std::vector<asset_load_info> m_new_assets_to_load;
  // synchronous tasks associated with this asset e.g. submit texture mem to GPU
  std::vector<asset_load_callback> m_asset_load_sync_callbacks;
};

class asset_manager {
public:

  asset_manager();

  asset_handle  load_asset(const std::string &path, const asset_type &assetType,
                          asset_loaded_callback on_asset_loaded = nullptr);

  void          unload_asset(const asset_handle &handle);

  asset *       get_asset(asset_handle &handle);

  template<typename _Ty, asset_type _AssetType>
  asset_handle  provide_asset(const std::string& name, _Ty data)
  {
    asset_t<_Ty,_AssetType>* to_asset = new asset_t<_Ty, _AssetType>(data, name);
    p_loaded_assets.emplace(
        to_asset->m_handle, std::move(std::unique_ptr<asset>(to_asset)));
    return to_asset->m_handle;
  }

  template <typename _Ty, asset_type _AssetType>
  asset_t<_Ty, _AssetType> *get_asset(asset_handle &handle) {
    asset *ass = get_asset(handle);
    return static_cast<asset_t<_Ty, _AssetType> *>(ass);
  }

  template <typename _Ty, asset_type _AssetType>
  asset_t<_Ty, _AssetType> *get_asset(const std::string &path) {
    asset_handle handle(path, _AssetType);
    asset *ass = get_asset(handle);
    return static_cast<asset_t<_Ty, _AssetType> *>(ass);
  }

  asset_load_progress get_asset_load_progress(const asset_handle &handle);

  bool any_assets_loading();
  bool any_assets_unloading();

  void wait_all_assets();
  void wait_all_unloads();
  void unload_all_assets();

  void update();
  void shutdown();

  // todo: make private after debugging and testing
  // protected:
  std::unordered_map<asset_handle, std::future<asset_load_return>>
      p_pending_load_tasks;
  std::unordered_map<asset_handle, std::unique_ptr<asset>>
      p_loaded_assets;
  std::unordered_map<asset_handle, asset_load_return>
      p_pending_load_callbacks;
  std::unordered_map<asset_handle, asset_unload_callback>
      p_pending_unload_callbacks;
  std::unordered_map<asset_handle, asset_loaded_callback>
      p_asset_loaded_callbacks;
  std::vector<asset_load_info>
      p_queued_loads;

  std::unique_ptr<efsw::FileWatcher>  p_file_watcher;
  std::unique_ptr<GemFileListener>    p_gem_listener;

  const uint16_t p_callback_tasks_per_tick = 1;
  const uint16_t p_max_async_tasks_in_flight = 8;

  void handle_load_and_unload_callbacks();

  void handle_pending_loads();

  void handle_async_tasks();

  void dispatch_asset_load_task(const asset_handle &handle,
                                asset_load_info &info);

private:
  void transition_asset_to_loaded(const asset_handle &handle,
                                  asset *asset_to_transition);
};
} // namespace gem