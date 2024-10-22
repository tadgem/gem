#include "asset.h"
#include "hash_string.h"
asset::asset(const std::string& path, asset_type type) : m_path(path), m_handle {type, hash_utils::get_string_hash(path)}
{
}

std::string get_asset_type_name(const asset_type& t)
{
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
