#pragma once
#include "gem/asset.h"
#include "gem/asset_hot_reload.h"
#include <functional>
#include <future>
#include <map>
#include <memory>

namespace gem {

using AssetLoadCallback = void (*)(AssetIntermediate *);
using AssetLoadedCallback = std::function<void(Asset *)>;
using AssetUnloadCallback = void (*)(Asset *);

enum class AssetLoadProgress { not_loaded, loading, loaded, unloading };

struct AssetLoadInfo {
  std::string m_path;
  AssetType m_type;

  bool operator==(const AssetLoadInfo &o) const {
    return m_path == o.m_path && m_type == o.m_type;
  }

  bool operator<(const AssetLoadInfo &o) const {
    return m_path.size() < o.m_path.size();
  }

  AssetHandle to_handle();

  GEM_IMPL_ALLOC(AssetLoadInfo)
};

struct AssetLoadResult {
  AssetIntermediate *m_loaded_asset_intermediate = nullptr;
  // additional assets that may be required to completely load this asset
  std::vector<AssetLoadInfo> m_new_assets_to_load;
  // synchronous tasks associated with this asset e.g. submit texture mem to GPU
  std::vector<AssetLoadCallback> m_asset_load_sync_callbacks;

  GEM_IMPL_ALLOC(AssetLoadResult)
};

class AssetManager {
public:
  AssetManager();

  AssetHandle LoadAsset(const std::string &path, const AssetType &assetType,
                         AssetLoadedCallback on_asset_loaded = nullptr);

  void UnloadAsset(const AssetHandle &handle);

  Asset *GetAsset(AssetHandle &handle);

  template <typename _Ty, AssetType _AssetType>
  AssetHandle provide_asset(const std::string &name, _Ty data) {
    TAsset<_Ty, _AssetType> *to_asset =
        new TAsset<_Ty, _AssetType>(data, name);
    p_loaded_assets.emplace(to_asset->m_handle,
                            std::move(std::unique_ptr<Asset>(to_asset)));
    return to_asset->m_handle;
  }

  template <typename _Ty, AssetType _AssetType>
  TAsset<_Ty, _AssetType> *GetAsset(AssetHandle &handle) {
    Asset *ass = GetAsset(handle);
    return static_cast<TAsset<_Ty, _AssetType> *>(ass);
  }

  template <typename _Ty, AssetType _AssetType>
  TAsset<_Ty, _AssetType> *GetAsset(const std::string &path) {
    AssetHandle handle(path, _AssetType);
    Asset *ass = GetAsset(handle);
    return static_cast<TAsset<_Ty, _AssetType> *>(ass);
  }

  AssetLoadProgress GetLoadProgress(const AssetHandle &handle);

  bool AnyAssetsLoading();
  bool AnyAssetsUnloading();

  void WaitAllLoads();
  void WaitAllUnloads();
  void UnloadAll();

  void Update();
  void Shutdown();

  void OnImGui();

  GEM_IMPL_ALLOC(AssetManager)

protected:
  std::unordered_map<AssetHandle, std::future<AssetLoadResult>>
      p_pending_load_tasks;
  std::unordered_map<AssetHandle, std::unique_ptr<Asset>> p_loaded_assets;
  std::unordered_map<AssetHandle, AssetLoadResult> p_pending_load_callbacks;
  std::unordered_map<AssetHandle, AssetUnloadCallback>
      p_pending_unload_callbacks;
  std::unordered_map<AssetHandle, AssetLoadedCallback>
      p_asset_loaded_callbacks;
  std::vector<AssetLoadInfo> p_queued_loads;

  std::unique_ptr<efsw::FileWatcher> p_file_watcher;
  std::unique_ptr<GemFileWatchListener> p_gem_listener;

  const uint16_t p_callback_tasks_per_tick = 1;
  const uint16_t p_max_async_tasks_in_flight = 8;

  void DispatchAssetTasksInternal();

  void HandlePendingLoadTasksInternal();

  void HandlePendingAsyncTasksInternal();

  void DispatchLoadTask(const AssetHandle &handle, AssetLoadInfo &info);

private:
  void FinalizeAssetLoad(const AssetHandle &handle,
                                  Asset *asset_to_transition);
};
} // namespace gem