#pragma once
#include <string>
#include "alias.h"
#include "hash_string.h"
enum class asset_type  {
    model,
    texture,
    audio,
    text,
    binary
};

std::string get_asset_type_name(const asset_type& t);

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
        std::cout << "Destructor called for asset_t<" << hash_utils::get_type_name<_Ty>() << "\n";
    }
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

    asset_t_intermediate(asset* data, _IntermediateType inter, const std::string& path) : asset_intermediate(data), m_intermediate(inter) {}

    ~asset_t_intermediate()
    {
        std::cout << "Destructor called for asset_t_intermediate<" << hash_utils::get_type_name<_AssetType>() << "\n";
    }

    asset_t<_AssetType, _AssetTypeEnum>* get_concrete_asset()
    {
        return static_cast<asset_t<_AssetType, _AssetTypeEnum>*>(m_asset_data);
    }

};

template<>
struct std::hash<asset_handle> {
    std::size_t operator()(const asset_handle& ah) const {
        return std::hash<u32>()(ah.m_path_hash) ^ std::hash<u32>()(static_cast<u32>(ah.m_type));
    }
};