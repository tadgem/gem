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