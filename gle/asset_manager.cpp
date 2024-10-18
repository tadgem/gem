#include "asset_manager.h"
#include "utils.h"
#include "model.h"

asset_handle asset_manager::load_asset(const std::string& path, const asset_type& assetType)
{
	asset_handle handle{ assetType, get_string_hash(path)};
	p_queued_loads.push_back(asset_load_info{ path, assetType });
	return handle;
}

void asset_manager::unload_asset(const asset_handle& handle)
{
	// todo:
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

    handle_load_and_unload_callbacks();

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

        if (asyncReturn.m_asset_load_tasks.empty()) {
            p_loaded_assets.emplace(handle, std::move(std::unique_ptr<asset>(asyncReturn.m_loaded_asset)));
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
            if (i >= asset.m_asset_load_tasks.size()) break;
            asset.m_asset_load_tasks.back()(asset.m_loaded_asset);
            asset.m_asset_load_tasks.pop_back();
            processedCallbacks++;
        }

        if (asset.m_asset_load_tasks.empty()) {
            clears.push_back(handle);
        }
    }
    for (auto& handle : clears) {
        p_loaded_assets.emplace(handle, std::move(std::unique_ptr<asset>(p_pending_load_callbacks[handle].m_loaded_asset)));
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

asset_load_return load_model_asset_manager(const std::string& path)
{
    std::vector < model::texture_entry> associated_textures;
    // move this into a heap allocated object
    model* m = new model(std::move(model::load_model_from_path_entries(path, associated_textures)));
    asset_t<model, asset_type::model>* model_asset = new asset_t<model, asset_type::model>(m, path);

    std::vector<asset_load_info> new_assets_to_load{};
    for (auto& tex : associated_textures)
    {
        new_assets_to_load.push_back(asset_load_info {tex.m_path, asset_type::texture});
    }

    asset_load_return ret{};
    ret.m_loaded_asset = static_cast<asset*>(model_asset);
    ret.m_asset_load_tasks = {};
    ret.m_new_assets_to_load = new_assets_to_load;
    return ret;
}

void asset_manager::dispatch_asset_load_task(const asset_handle& handle, asset_load_info& info)
{
    switch (info.m_type)
    {
    case asset_type::model:
        p_pending_load_tasks.emplace(handle, std::move(std::async(std::launch::async, load_model_asset_manager, info.m_path)));
        break;
    case asset_type::texture:
        break;
    }
}

