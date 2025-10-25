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

enum class AssetLoadProgress { kNotLoaded, kLoading, kLoaded, kUnloading };

struct AssetLoadInfo {
  std::string path;
  AssetType asset_type;

  bool operator==(const AssetLoadInfo &o) const {
    return path == o.path && asset_type == o.asset_type;
  }

  bool operator<(const AssetLoadInfo &o) const {
    return path.size() < o.path.size();
  }

  AssetHandle to_handle();

  GEM_IMPL_ALLOC(AssetLoadInfo)
};

struct AssetLoadResult {
  AssetIntermediate *loaded_asset_intermediate = nullptr;
  // additional assets that may be required to completely load this asset
  std::vector<AssetLoadInfo> new_assets_to_load;
  // synchronous tasks associated with this asset e.g. submit texture mem to GPU
  std::vector<AssetLoadCallback> sync_load_tasks;

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
    loaded_assets_.emplace(to_asset->handle,
                            std::move(std::unique_ptr<Asset>(to_asset)));
    return to_asset->handle;
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
      pending_async_load_tasks_;
  std::unordered_map<AssetHandle, std::unique_ptr<Asset>> loaded_assets_;
  std::unordered_map<AssetHandle, AssetLoadResult> pending_sync_load_tasks_;
  std::unordered_map<AssetHandle, AssetUnloadCallback>
      pending_unload_tasks_;
  std::unordered_map<AssetHandle, AssetLoadedCallback>
      asset_loaded_callbacks_;
  std::vector<AssetLoadInfo> enqueued_loads_;

  std::unique_ptr<efsw::FileWatcher> file_watcher_;
  std::unique_ptr<GemFileWatchListener> gem_listener_;

  const uint16_t kSyncTasksPerFrame = 1;
  const uint16_t kMaxAsyncTasksInFlight = 8;

  void DispatchAssetTasksInternal();

  void HandlePendingLoadTasksInternal();

  void HandlePendingAsyncTasksInternal();

  void DispatchLoadTask(const AssetHandle &handle, AssetLoadInfo &info);

private:
  void FinalizeAssetLoad(const AssetHandle &handle,
                                  Asset *asset_to_transition);
};
} // namespace gem