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
  AssetType m_type;
  HashString m_path_hash;

  AssetHandle(){};
  AssetHandle(const std::string &path, AssetType type);
  AssetHandle(const HashString &path_hash, AssetType type);

  bool operator==(const AssetHandle &o) const {
    return m_type == o.m_type && m_path_hash == o.m_path_hash;
  }

  bool operator<(const AssetHandle &o) const {
    return m_type < o.m_type && m_path_hash < o.m_path_hash;
  }

  static AssetHandle INVALID() {
    return AssetHandle(HashString(UINT64_MAX), AssetType::COUNT);
  }

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(AssetHandle, m_type, m_path_hash);

  GEM_IMPL_ALLOC(AssetHandle)
};

struct SerializableAssetHandle {
  AssetHandle m_handle;
  std::string m_path;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(SerializableAssetHandle, m_handle, m_path)

  GEM_IMPL_ALLOC(SerializableAssetHandle)
};

class Asset {
public:
  Asset(const std::string &path, AssetType type);

  virtual ~Asset(){};

  GEM_IMPL_ALLOC(Asset)

  const std::string m_path;
  const AssetHandle m_handle;
};

template <typename _Ty, AssetType _AssetType> class TAsset : public Asset {
public:
  _Ty m_data;
  TAsset(_Ty data, const std::string &path)
      : Asset(path, _AssetType), m_data(data) {}

  ~TAsset() {}

  void *operator new(size_t size) {
    std::string type_name = HashUtils::get_type_name<TAsset<_Ty, _AssetType>>();
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
    std::string type_name = HashUtils::get_type_name<TAsset<_Ty, _AssetType>>();
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
  Asset *m_asset_data;

  AssetIntermediate(Asset *asset) : m_asset_data(asset){};

  virtual ~AssetIntermediate(){};
};

template <typename _AssetType, typename _IntermediateType,
          AssetType _AssetTypeEnum>
class TAssetIntermediate : public AssetIntermediate {
public:
  _IntermediateType m_intermediate;
  std::string m_path;

  TAssetIntermediate(Asset *data, const _IntermediateType &inter,
                       const std::string &path)
      : AssetIntermediate(data), m_intermediate(inter), m_path(path) {}


  TAsset<_AssetType, _AssetTypeEnum> *get_concrete_asset() {
    return static_cast<TAsset<_AssetType, _AssetTypeEnum> *>(m_asset_data);
  }

  void *operator new(size_t size) {
    std::string type_name = HashUtils::get_type_name<
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
    std::string type_name = HashUtils::get_type_name<
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
    return std::hash<u64>()(ah.m_path_hash) ^
           std::hash<u32>()(static_cast<u32>(ah.m_type));
  }
};
