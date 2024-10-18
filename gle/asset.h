#pragma once
#include <string>
#include "alias.h"

enum class asset_type  {
    model,
    texture,
    audio,
    text,
    binary
};

struct asset_handle
{
    asset_type  m_type;
    u64         m_path_hash;

    bool operator==(const asset_handle& o) const {
        return m_type == o.m_type && m_path_hash == o.m_path_hash;
    }

    bool operator<(const asset_handle& o) const {
        return m_type < o.m_type && m_path_hash < o.m_path_hash;
    }
};

class asset
{
public:
    asset(const std::string& path, asset_type type);

    const std::string   m_path;
    const asset_handle  m_handle;
};

template<typename _Ty, asset_type _AssetType>
class asset_t : public asset
{
public:
    _Ty*    m_data;
    asset_t(const _Ty* data, const std::string& path) : asset(path, _AssetType), m_data(data) {}
};

/* required to hash a container */
template<>
struct std::hash<asset_handle> {
    std::size_t operator()(const asset_handle& ah) const {
        return std::hash<u32>()(ah.m_path_hash) ^ std::hash<u32>()(static_cast<u32>(ah.m_type));
    }
};