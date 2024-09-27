#include "asset.h"
#include "hash_string.h"
asset::asset(const std::string& path, asset_type type) : m_path(path), m_handle {type, get_string_hash(path)}
{
}
