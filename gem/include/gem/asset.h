#pragma once
#include "alias.h"
#include "dbg_memory.h"
#include "hash_string.h"
#include "spdlog/spdlog.h"
#include <string>

namespace gem {

enum class AssetType : u32 {
  model,
  texture,
  shader,
  audio,
  text,
  binary,
  COUNT
};

std::string get_asset_type_name(const AssetType &t);

struct AssetHandle {
  AssetType asset_type;
  HashString path_hash;

  AssetHandle() : asset_type(AssetType::COUNT), path_hash(0) {};
  AssetHandle(const std::string &path, AssetType type);
  AssetHandle(const HashString &path_hash, AssetType type);

  bool operator==(const AssetHandle &o) const {
    return asset_type == o.asset_type && path_hash == o.path_hash;
  }

  bool operator<(const AssetHandle &o) const {
    return asset_type < o.asset_type && path_hash < o.path_hash;
  }

  static AssetHandle INVALID() {
    return AssetHandle(HashString(UINT64_MAX), AssetType::COUNT);
  }

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(AssetHandle, asset_type, path_hash);

  GEM_IMPL_ALLOC(AssetHandle)
};

struct SerializableAssetHandle {
  AssetHandle handle;
  std::string path;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(SerializableAssetHandle, handle, path)

  GEM_IMPL_ALLOC(SerializableAssetHandle)
};

class Asset {
public:
  Asset(const std::string &path, AssetType type);

  virtual ~Asset(){};

  GEM_IMPL_ALLOC(Asset)

  const std::string path;
  const AssetHandle handle;
};

template <typename _Ty, AssetType _AssetType> class TAsset : public Asset {
public:
  _Ty data;
  TAsset(_Ty data, const std::string &path)
      : Asset(path, _AssetType), data(data) {}

  ~TAsset() {}

  void *operator new(size_t size) {
    std::string type_name = HashUtils::GetTypeName<TAsset<_Ty, _AssetType>>();
    if (DebugMemoryTracker::s_instance->s_allocation_info.find(type_name) ==
        DebugMemoryTracker::s_instance->s_allocation_info.end()) {
      DebugMemoryTracker::s_instance->s_allocation_info.emplace(
          type_name, DebugAllocInfo{0, 0});
    }
    DebugMemoryTracker::s_instance->s_allocation_info[type_name].count++;
    DebugMemoryTracker::s_instance->s_allocation_info[type_name].size += size;
    return malloc(size);
  };

  void operator delete(void *p) {
    std::string type_name = HashUtils::GetTypeName<TAsset<_Ty, _AssetType>>();
    free(p);
    if (!DebugMemoryTracker::s_instance)
      return;
    DebugMemoryTracker::s_instance->s_allocation_info[type_name].count--;
    DebugMemoryTracker::s_instance->s_allocation_info[type_name].size -=
        sizeof(TAsset<_Ty, _AssetType>);
  };
};

class AssetIntermediate {
public:
  Asset *asset_data;

  AssetIntermediate(Asset *asset) : asset_data(asset){};

  virtual ~AssetIntermediate(){};
};

template <typename _AssetType, typename _IntermediateType,
          AssetType _AssetTypeEnum>
class TAssetIntermediate : public AssetIntermediate {
public:
  _IntermediateType intermediate;
  std::string path;

  TAssetIntermediate(Asset *data, const _IntermediateType &inter,
                       const std::string &path)
      : AssetIntermediate(data), intermediate(inter), path(path) {}


  TAsset<_AssetType, _AssetTypeEnum> *get_concrete_asset() {
    return static_cast<TAsset<_AssetType, _AssetTypeEnum> *>(asset_data);
  }

  void *operator new(size_t size) {
    std::string type_name = HashUtils::GetTypeName<
        TAssetIntermediate<_AssetType, _IntermediateType, _AssetTypeEnum>>();
    if (DebugMemoryTracker::s_instance->s_allocation_info.find(type_name) ==
        DebugMemoryTracker::s_instance->s_allocation_info.end()) {
      DebugMemoryTracker::s_instance->s_allocation_info.emplace(
          type_name, DebugAllocInfo{0, 0});
    }
    DebugMemoryTracker::s_instance->s_allocation_info[type_name].count++;
    DebugMemoryTracker::s_instance->s_allocation_info[type_name].size += size;
    return malloc(size);
  };

  void operator delete(void *p) {
    std::string type_name = HashUtils::GetTypeName<
        TAssetIntermediate<_AssetType, _IntermediateType, _AssetTypeEnum>>();
    free(p);
    if (!DebugMemoryTracker::s_instance)
      return;
    DebugMemoryTracker::s_instance->s_allocation_info[type_name].count--;
    DebugMemoryTracker::s_instance->s_allocation_info[type_name]
        .size -= sizeof(
            TAssetIntermediate<_AssetType, _IntermediateType, _AssetTypeEnum>);
  };
};
} // namespace gem

template <> struct std::hash<gem::AssetHandle> {
  std::size_t operator()(const gem::AssetHandle &ah) const {
    return std::hash<u64>()(ah.path_hash) ^
           std::hash<u32>()(static_cast<u32>(ah.asset_type));
  }
};
