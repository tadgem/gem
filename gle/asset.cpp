#include "asset.h"
#include "hash_string.h"
#include "profile.h"

asset::asset(const std::string& path, asset_type type) : m_path(path), m_handle (path, type)
{
	ZoneScoped;
}

std::string get_asset_type_name(const asset_type& t)
{
	ZoneScoped;
	switch (t)
	{
	case asset_type::model:
		return "model";
	case asset_type::text:
		return "text";
	case asset_type::texture:
		return "texture";
	case asset_type::shader:
		return "shader";
	}

	return "unknown";
}


asset_handle::asset_handle(const std::string& path, asset_type type)
{
	ZoneScoped;
	m_type = type;
	m_path_hash = hash_utils::get_string_hash(path);
}

asset_handle::asset_handle(const hash_string& path_hash, asset_type type)
{
	ZoneScoped;
	m_type = type;
	m_path_hash = path_hash;
}
