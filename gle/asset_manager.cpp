#include "asset_manager.h"
#include "utils.h"
#include "model.h"
#include "hash_string.h"
#include <iostream>
#include <filesystem>

using texture_intermediate_asset = asset_t_intermediate<texture, std::vector<unsigned char>, asset_type::texture>;
using model_intermediate_asset = asset_t_intermediate<model, std::vector<model::mesh_entry>, asset_type::model>;

asset_handle asset_manager::load_asset(const std::string& path, const asset_type& assetType)
{
    std::string wd = std::filesystem::current_path().string();
    std::string tmp_path = path;
    if(path.find(wd) != std::string::npos)
    {
        tmp_path.erase(tmp_path.find(wd), wd.length());

        for (int i = 0; i < 2; i++)
        {
            if (tmp_path[0] != '\\' && tmp_path[0] != '/')
            {
                break;
            }
            tmp_path.erase(0, 1);
        }
    }

	asset_handle handle{ assetType, hash_utils::get_string_hash(path)};
	p_queued_loads.push_back(asset_load_info{ path, assetType });
	return handle;
}

asset* asset_manager::get_asset(asset_handle& handle)
{
	if (p_loaded_assets.find(handle) == p_loaded_assets.end()) {
		return nullptr;
	}

	return p_loaded_assets[handle].get();
}

asset_load_progress asset_manager::get_asset_load_progress(const asset_handle& handle)
{
    for (auto& queued : p_queued_loads) {
        if (queued.to_handle() == handle) {
            return asset_load_progress::loading;
        }
    }

    if (p_pending_load_tasks.find(handle) != p_pending_load_tasks.end() ||
        p_pending_load_callbacks.find(handle) != p_pending_load_callbacks.end()) {
        return asset_load_progress::loading;
    }

    if (p_pending_unload_callbacks.find(handle) != p_pending_unload_callbacks.end()) {
        return asset_load_progress::unloading;
    }

    if (p_loaded_assets.find(handle) != p_loaded_assets.end()) {
        return asset_load_progress::loaded;
    }

    return asset_load_progress::not_loaded;
}

bool asset_manager::any_assets_loading()
{
    return !p_pending_load_tasks.empty() || !p_pending_load_callbacks.empty() || !p_pending_unload_callbacks.empty() ||
        !p_queued_loads.empty();
}

bool asset_manager::any_assets_unloading()
{
    return !p_pending_unload_callbacks.empty();
}

void asset_manager::wait_all_assets()
{
    while (any_assets_loading())
    {
        update();
    }
}

void asset_manager::wait_all_unloads()
{
    while (any_assets_unloading())
    {
        update();
    }
}

void asset_manager::unload_all_assets()
{
    std::vector<asset_handle> assetsRemaining{};

    for (auto& [handle, asset] : p_loaded_assets) {
        assetsRemaining.push_back(handle);
    }

    for (auto& handle : assetsRemaining) {
        unload_asset(handle);
    }

    wait_all_unloads();
}

void asset_manager::update()
{
    if (!any_assets_loading()) {
        return;
    }

    // process any synchronous tasks that must be performed
    handle_load_and_unload_callbacks();

    // enqueue pending loads
    handle_pending_loads();

    std::vector<asset_handle> finished;
    for (auto& [handle, future] : p_pending_load_tasks) {
        if (utils::is_future_ready(future)) {
            finished.push_back(handle);
        }
    }

    for (auto& handle : finished) {
        asset_load_return asyncReturn = p_pending_load_tasks[handle].get();
        // enqueue new loads
        for (auto& newLoad : asyncReturn.m_new_assets_to_load) {
            load_asset(newLoad.m_path, newLoad.m_type);
        }

        if (asyncReturn.m_asset_load_sync_callbacks.empty()) {
            p_loaded_assets.emplace(handle, std::move(std::unique_ptr<asset>(asyncReturn.m_loaded_asset_intermediate->m_asset_data)));
            delete asyncReturn.m_loaded_asset_intermediate;
        }
        else {
            p_pending_load_callbacks.emplace(handle, asyncReturn);
        }

        p_pending_load_tasks.erase(handle);
    }
}


void asset_manager::shutdown()
{
    wait_all_assets();
    wait_all_unloads();
    unload_all_assets();
}

void asset_manager::handle_load_and_unload_callbacks()
{
    u16 processedCallbacks = 0;
    std::vector<asset_handle> clears;

    for (auto& [handle, asset] : p_pending_load_callbacks) {
        if (processedCallbacks == p_callback_tasks_per_tick) break;

        for (u16 i = 0; i < p_callback_tasks_per_tick - processedCallbacks; i++) {
            if (i >= asset.m_asset_load_sync_callbacks.size()) break;
            asset.m_asset_load_sync_callbacks.back()(asset.m_loaded_asset_intermediate);
            asset.m_asset_load_sync_callbacks.pop_back();
            processedCallbacks++;
        }

        if (asset.m_asset_load_sync_callbacks.empty()) {
            clears.push_back(handle);
        }
    }
    for (auto& handle : clears) {
        p_loaded_assets.emplace(handle, std::move(std::unique_ptr<asset>(p_pending_load_callbacks[handle].m_loaded_asset_intermediate->m_asset_data)));
        delete p_pending_load_callbacks[handle].m_loaded_asset_intermediate;
        p_pending_load_callbacks.erase(handle);
    }
    clears.clear();

    for (auto& [handle, callback] : p_pending_unload_callbacks) {
        if (processedCallbacks == p_callback_tasks_per_tick) break;
        callback(p_loaded_assets[handle].get());
        clears.push_back(handle);
        processedCallbacks++;
    }

    for (auto& handle : clears) {
        p_pending_unload_callbacks.erase(handle);
        p_loaded_assets[handle].reset();
        p_loaded_assets.erase(handle);
    }
}

void asset_manager::handle_pending_loads()
{
    while (p_pending_load_tasks.size() < p_max_async_tasks_in_flight && !p_queued_loads.empty()) {
        auto& info = p_queued_loads.front();
        dispatch_asset_load_task(info.to_handle(), info);
        p_queued_loads.erase(p_queued_loads.begin());
    }
}

void submit_meshes_to_gpu(asset_intermediate* model_asset)
{
    // cast to model asset
    model_intermediate_asset* inter = static_cast<model_intermediate_asset*>(model_asset);
    asset_t<model, asset_type::model>* ma = inter->get_concrete_asset();
    // go through each entry
    model::mesh_entry& entry = inter->m_intermediate.front();
    vao_builder mesh_builder{};
    mesh_builder.begin();
    std::vector<float> buffer;
    for (int i = 0; i < entry.m_positions.size(); i++)
    {
        buffer.push_back(entry.m_positions[i].x);
        buffer.push_back(entry.m_positions[i].y);
        buffer.push_back(entry.m_positions[i].z);
        buffer.push_back(entry.m_normals[i].x);
        buffer.push_back(entry.m_normals[i].y);
        buffer.push_back(entry.m_normals[i].z);
        buffer.push_back(entry.m_uvs[i].x);
        buffer.push_back(entry.m_uvs[i].y);
    }

    mesh_builder.add_vertex_buffer(buffer);
    mesh_builder.add_vertex_attribute(0, 8 * sizeof(float), 3);
    mesh_builder.add_vertex_attribute(1, 8 * sizeof(float), 3);
    mesh_builder.add_vertex_attribute(2, 8 * sizeof(float), 2);

    std::vector<u32> indices;
    for (int i = 0; i < entry.m_indices.size(); i++)
    {
        indices.push_back(entry.m_indices[i]);
    }

    mesh_builder.add_index_buffer(indices);
    mesh m{};
    m.m_index_count = indices.size();
    m.m_material_index = entry.m_material_index;
    m.m_original_aabb = entry.m_mesh_aabb;
    m.m_vao = mesh_builder.build();
    // create real mesh object on gpu

    ma->m_data.m_meshes.push_back(m);
    inter->m_intermediate[0].free();
    inter->m_intermediate.erase(inter->m_intermediate.begin());
}

void submit_texture_to_gpu(asset_intermediate* texture_asset)
{
    // cast to model asset
    texture_intermediate_asset* ta_inter = static_cast<texture_intermediate_asset*>(texture_asset);
    asset_t<texture, asset_type::texture>* ta = ta_inter->get_concrete_asset();

    if (ta->m_path.find("dds") != std::string::npos)
    {
        ta->m_data.load_texture_gli(ta_inter->m_intermediate);
    }
    else {
        ta->m_data.load_texture_stbi(ta_inter->m_intermediate);
    }

    ta_inter->m_intermediate.clear();
}

asset_load_return load_model_asset_manager(const std::string& path)
{
    std::vector < model::texture_entry> associated_textures;
    std::vector <model::mesh_entry> mesh_entries;
    // move this into a heap allocated object
    model m = model::load_model_from_path_entries(path, associated_textures, mesh_entries);
    asset_t<model, asset_type::model>* model_asset = new asset_t<model, asset_type::model>(m, path);
    model_intermediate_asset* model_intermediate = new model_intermediate_asset(model_asset, mesh_entries, path);
    std::string directory = utils::get_directory_from_path(path);

    asset_load_return ret{};
    for (auto& tex : associated_textures)
    {
        std::string final_path = directory + "/" + tex.m_path;
        ret.m_new_assets_to_load.push_back(asset_load_info {final_path, asset_type::texture});
    }
    for (int i = 0; i < m.m_meshes.size(); i++)
    {
        ret.m_asset_load_sync_callbacks.push_back(submit_meshes_to_gpu);
    }
    model_intermediate->m_asset_data = model_asset;
    ret.m_loaded_asset_intermediate = model_intermediate;
    return ret;
}

void unload_model_asset_manager(asset* _asset)
{
    asset_t<model, asset_type::model>* ma = static_cast<asset_t<model, asset_type::model>*>(_asset);
    ma->m_data.free();
}

asset_load_return load_texture_asset_manager(const std::string& path)
{
    asset_load_return ret{};
    ret.m_new_assets_to_load = {};
    ret.m_asset_load_sync_callbacks.push_back(submit_texture_to_gpu);

    texture t{};
    std::vector<unsigned char> binary = utils::load_binary_from_path(path);
    asset_t<texture, asset_type::texture>* ta = new asset_t<texture, asset_type::texture>(t, path);
    texture_intermediate_asset* ta_inter = new texture_intermediate_asset(ta, binary, path);

    ret.m_loaded_asset_intermediate = ta_inter;
    return ret;
}

void unload_texture_asset_manager(asset* _asset)
{
    asset_t<texture, asset_type::texture>* ta = static_cast<asset_t<texture, asset_type::texture>*>(_asset);
    ta->m_data.free();
}

void asset_manager::dispatch_asset_load_task(const asset_handle& handle, asset_load_info& info)
{
    switch (info.m_type)
    {
    case asset_type::model:
        p_pending_load_tasks.emplace(handle, std::move(std::async(std::launch::async, load_model_asset_manager, info.m_path)));
        break;
    case asset_type::texture:
        p_pending_load_tasks.emplace(handle, std::move(std::async(std::launch::async, load_texture_asset_manager, info.m_path)));
        break;
    }
}

asset_handle asset_load_info::to_handle()
{
    return asset_handle{ m_type, hash_utils::get_string_hash(m_path) };
}

void asset_manager::unload_asset(const asset_handle& handle)
{
    if (p_loaded_assets.find(handle) == p_loaded_assets.end()) {
        return;
    }

    switch (handle.m_type) {
    case asset_type::model:
        p_pending_unload_callbacks.emplace(handle, unload_model_asset_manager);
        break;
    case asset_type::texture:
        p_pending_unload_callbacks.emplace(handle, unload_texture_asset_manager);
        break;
    default:
        break;
    }
}