#pragma once
#include <string>
#include "alias.h"
#include "hash_string.h"
#include "dbg_memory.h"
#include "spdlog/spdlog.h"
#include "serialization.h"

enum class asset_type  {
    model,
    texture,
    shader,
    audio,
    text,
    binary,
    COUNT
};

std::string get_asset_type_name(const asset_type& t);

struct asset_handle
{
    asset_type  m_type;
    hash_string m_path_hash;

    asset_handle() {};
    asset_handle(const std::string& path, asset_type type);
    asset_handle(const hash_string& path_hash, asset_type type);

    bool operator==(const asset_handle& o) const {
        return m_type == o.m_type && m_path_hash == o.m_path_hash;
    }

    bool operator<(const asset_handle& o) const {
        return m_type < o.m_type && m_path_hash < o.m_path_hash;
    }

    static asset_handle INVALID()
    {
        return asset_handle(hash_string(UINT64_MAX), asset_type::COUNT );
    }

    template<typename Archive>
    void serialize(Archive& ar) {
        ZoneScoped;
        ar(m_type, m_path_hash);
    }
};

struct serializable_asset_handle
{
    asset_handle    m_handle;
    std::string     m_path;

    template<typename Archive>
    void serialize(Archive& ar) {
        ZoneScoped;
        ar(m_handle, m_path);
    }
};

class asset
{
public:
    asset(const std::string& path, asset_type type);

    virtual ~asset() {};

    const std::string   m_path;
    const asset_handle  m_handle;
};

template<typename _Ty, asset_type _AssetType>
class asset_t : public asset
{
public:
    _Ty    m_data;
    asset_t(_Ty data, const std::string& path) : asset(path, _AssetType), m_data(data) {}

    ~asset_t()
    {
        m_data.~_Ty();
    }

    void* operator new(size_t size) {
        std::string type_name = hash_utils::get_type_name<asset_t<_Ty, _AssetType>>();
        if (debug_memory_tracker::s_instance->s_allocation_info.find(type_name) == debug_memory_tracker::s_instance->s_allocation_info.end()) {
            debug_memory_tracker::s_instance->s_allocation_info.emplace(type_name, alloc_info{ 0,0 });
        } debug_memory_tracker::s_instance->s_allocation_info[type_name].count++; debug_memory_tracker::s_instance->s_allocation_info[type_name].size += size; return malloc(size);
    };

    void operator delete(void* p) {
        std::string type_name = hash_utils::get_type_name<asset_t<_Ty, _AssetType>>();
        free(p); if (!debug_memory_tracker::s_instance) return; debug_memory_tracker::s_instance->s_allocation_info[type_name].count--; debug_memory_tracker::s_instance->s_allocation_info[type_name].size -= sizeof(asset_t<_Ty, _AssetType>);
    };
};

class asset_intermediate
{
public:
    asset* m_asset_data;

    asset_intermediate(asset* asset) : m_asset_data(asset) {};

    virtual ~asset_intermediate() {};

};

template<typename _AssetType, typename _IntermediateType, asset_type _AssetTypeEnum>
class asset_t_intermediate : public asset_intermediate
{
public:
    _IntermediateType   m_intermediate;
    std::string         m_path;

    asset_t_intermediate(asset* data, const _IntermediateType& inter, const std::string& path) : asset_intermediate(data), m_intermediate(inter), m_path(path) {}

    ~asset_t_intermediate()
    {
        m_intermediate.~_IntermediateType();
    }

    asset_t<_AssetType, _AssetTypeEnum>* get_concrete_asset()
    {
        return static_cast<asset_t<_AssetType, _AssetTypeEnum>*>(m_asset_data);
    }

    void* operator new(size_t size) {
        std::string type_name = hash_utils::get_type_name<asset_t_intermediate<_AssetType, _IntermediateType, _AssetTypeEnum>>();
        if (debug_memory_tracker::s_instance->s_allocation_info.find(type_name) == debug_memory_tracker::s_instance->s_allocation_info.end()) {
            debug_memory_tracker::s_instance->s_allocation_info.emplace(type_name, alloc_info{ 0,0 });
        } debug_memory_tracker::s_instance->s_allocation_info[type_name].count++; debug_memory_tracker::s_instance->s_allocation_info[type_name].size += size; return malloc(size);
    };

    void operator delete(void* p) {
        std::string type_name = hash_utils::get_type_name<asset_t_intermediate<_AssetType, _IntermediateType, _AssetTypeEnum>>();
        free(p); if (!debug_memory_tracker::s_instance) return; debug_memory_tracker::s_instance->s_allocation_info[type_name].count--; debug_memory_tracker::s_instance->s_allocation_info[type_name].size -= sizeof(asset_t_intermediate<_AssetType, _IntermediateType, _AssetTypeEnum>);
    };

};

template<>
struct std::hash<asset_handle> {
    std::size_t operator()(const asset_handle& ah) const {
        return std::hash<u32>()(ah.m_path_hash) ^ std::hash<u32>()(static_cast<u32>(ah.m_type));
    }
};