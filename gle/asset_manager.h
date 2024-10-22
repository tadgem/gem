#pragma once
#include <map>
#include <future>
#include <memory>
#include "asset.h"


using asset_load_callback = void(*) (asset_intermediate*);
using asset_unload_callback = void(*) (asset*);

enum class asset_load_progress {
    not_loaded,
    loading,
    loaded,
    unloading
};

struct asset_load_info {
    std::string m_path;
    asset_type m_type;

    bool operator==(const asset_load_info& o) const {
        return m_path == o.m_path && m_type == o.m_type;
    }

    bool operator<(const asset_load_info& o) const {
        return m_path.size() < o.m_path.size();
    }

    template<typename Archive>
    void serialize(Archive& ar) {
        ZoneScoped;
        ar(m_Path, m_Type);
    }

    asset_handle to_handle();
};

struct asset_load_return {
    asset_intermediate*                     m_loaded_asset_intermediate;
    // additional assets that may be required for this asset
    // e.g. textures for a model
    std::vector<asset_load_info>            m_new_assets_to_load;
    // tasks associated with this asset to be performed. 
    // e.g. submit mesh / texture to GPU. 
    std::vector<asset_load_callback>        m_asset_load_sync_callbacks;
};

class asset_manager {
public:

    asset_handle            load_asset(const std::string& path, const asset_type& assetType);
    void                    unload_asset(const asset_handle& handle);
    asset*                  get_asset(asset_handle& handle);

    asset_load_progress     get_asset_load_progress(const asset_handle& handle);
    bool                    any_assets_loading();
    bool                    any_assets_unloading();

    void                    wait_all_assets();
    void                    wait_all_unloads();
    void                    unload_all_assets();

    void                    update();
    void                    shutdown();
   
// todo: make private after debugging and testing
// protected:
    // Move the Asset* into a UPtr once returned from the future
    std::unordered_map<asset_handle, std::future<asset_load_return>>    p_pending_load_tasks;
    std::unordered_map<asset_handle, std::unique_ptr<asset>>            p_loaded_assets;
    std::unordered_map<asset_handle, asset_load_return>                 p_pending_load_callbacks;
    std::unordered_map<asset_handle, asset_unload_callback>             p_pending_unload_callbacks;
    std::vector<asset_load_info>                                        p_queued_loads;

    const uint16_t p_callback_tasks_per_tick = 1;
    const uint16_t p_max_async_tasks_in_flight = 2;

    void handle_load_and_unload_callbacks();

    void handle_pending_loads();

    void handle_async_tasks();

    void dispatch_asset_load_task(const asset_handle& handle, asset_load_info& info);
};