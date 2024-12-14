#define GLM_ENABLE_EXPERIMENTAL
#include "gem/asset_manager.h"
#include "ImFileDialog.h"
#include "gem/gl/gl_shader.h"
#include "gem/hash_string.h"
#include "gem/model.h"
#include "gem/profile.h"
#include "gem/utils.h"
#include "spdlog/spdlog.h"
#include <filesystem>

namespace gem {

using texture_intermediate_asset =
    TAssetIntermediate<Texture, std::vector<unsigned char>,
                         AssetType::texture>;
using model_intermediate_asset =
    TAssetIntermediate<Model, std::vector<Model::MeshEntry>,
                         AssetType::model>;
using shader_intermediate_asset =
    TAssetIntermediate<GLShader, std::string, AssetType::shader>;

AssetHandle AssetManager::load_asset(const std::string &path,
                                       const AssetType &assetType,
                                      AssetLoadedCallback on_asset_loaded) {
  ZoneScoped;
  if (!std::filesystem::exists(path)) {
    return AssetHandle::AssetHandle();
  }

  std::string wd = std::filesystem::current_path().string();
  std::string tmp_path = path;
  if (path.find(wd) != std::string::npos) {
    tmp_path.erase(tmp_path.find(wd), wd.length());

    for (int i = 0; i < 2; i++) {
      if (tmp_path[0] != '\\' && tmp_path[0] != '/') {
        break;
      }
      tmp_path.erase(0, 1);
    }
  }

  AssetHandle handle(tmp_path, assetType);
  AssetLoadInfo load_info{tmp_path, assetType};

  auto it = std::find(p_queued_loads.begin(), p_queued_loads.end(), load_info);

  for (auto &queued_load : p_queued_loads) {
    if (load_info == queued_load) {
      return queued_load.to_handle();
    }
  }

  p_queued_loads.push_back(load_info);

  if (on_asset_loaded != nullptr) {
    p_asset_loaded_callbacks.emplace(handle, on_asset_loaded);
  }

  return handle;
}

Asset *AssetManager::get_asset(AssetHandle &handle) {
  ZoneScoped;
  if (p_loaded_assets.find(handle) == p_loaded_assets.end()) {
    return nullptr;
  }

  return p_loaded_assets[handle].get();
}

AssetLoadProgress
AssetManager::get_asset_load_progress(const AssetHandle &handle) {
  ZoneScoped;
  for (auto &queued : p_queued_loads) {
    if (queued.to_handle() == handle) {
      return AssetLoadProgress::loading;
    }
  }

  if (p_pending_load_tasks.find(handle) != p_pending_load_tasks.end() ||
      p_pending_load_callbacks.find(handle) != p_pending_load_callbacks.end()) {
    return AssetLoadProgress::loading;
  }

  if (p_pending_unload_callbacks.find(handle) !=
      p_pending_unload_callbacks.end()) {
    return AssetLoadProgress::unloading;
  }

  if (p_loaded_assets.find(handle) != p_loaded_assets.end()) {
    return AssetLoadProgress::loaded;
  }

  return AssetLoadProgress::not_loaded;
}

bool AssetManager::any_assets_loading() {
  ZoneScoped;
  return !p_pending_load_tasks.empty() || !p_pending_load_callbacks.empty() ||
         !p_pending_unload_callbacks.empty() || !p_queued_loads.empty();
}

bool AssetManager::any_assets_unloading() {
  ZoneScoped;
  return !p_pending_unload_callbacks.empty();
}

void AssetManager::wait_all_assets() {
  ZoneScoped;
  while (any_assets_loading()) {
    update();
  }
}

void AssetManager::wait_all_unloads() {
  ZoneScoped;
  while (any_assets_unloading()) {
    update();
  }
}

void AssetManager::unload_all_assets() {
  ZoneScoped;
  std::vector<AssetHandle> assetsRemaining{};

  for (auto &[handle, asset] : p_loaded_assets) {
    assetsRemaining.push_back(handle);
  }

  for (auto &handle : assetsRemaining) {
    unload_asset(handle);
  }

  wait_all_unloads();
}

void AssetManager::update() {
  ZoneScoped;
  if (!any_assets_loading()) {
    return;
  }

  handle_load_and_unload_callbacks();
  handle_pending_loads();
  handle_async_tasks();
}

void AssetManager::shutdown() {
  ZoneScoped;
  wait_all_assets();
  wait_all_unloads();
  unload_all_assets();
}

void AssetManager::handle_load_and_unload_callbacks() {
  ZoneScoped;
  u16 processedCallbacks = 0;
  std::vector<AssetHandle> clears;

  for (auto &[handle, asset] : p_pending_load_callbacks) {
    if (processedCallbacks == p_callback_tasks_per_tick)
      break;

    for (u16 i = 0; i < p_callback_tasks_per_tick - processedCallbacks; i++) {
      if (i >= asset.m_asset_load_sync_callbacks.size())
        break;
      asset.m_asset_load_sync_callbacks.back()(
          asset.m_loaded_asset_intermediate);
      asset.m_asset_load_sync_callbacks.pop_back();
      processedCallbacks++;
    }

    if (asset.m_asset_load_sync_callbacks.empty()) {
      clears.push_back(handle);
    }
  }
  for (auto &handle : clears) {
    // p_loaded_assets.emplace(handle,
    // std::move(std::unique_ptr<asset>(p_pending_load_callbacks[handle].m_loaded_asset_intermediate->m_asset_data)));
    transition_asset_to_loaded(handle,
                               p_pending_load_callbacks[handle]
                                   .m_loaded_asset_intermediate->m_asset_data);
    delete p_pending_load_callbacks[handle].m_loaded_asset_intermediate;
    p_pending_load_callbacks.erase(handle);
  }
  clears.clear();

  for (auto &[handle, callback] : p_pending_unload_callbacks) {
    if (processedCallbacks == p_callback_tasks_per_tick)
      break;
    callback(p_loaded_assets[handle].get());
    clears.push_back(handle);
    processedCallbacks++;
  }

  for (auto &handle : clears) {
    p_pending_unload_callbacks.erase(handle);
    p_loaded_assets[handle].reset();
    p_loaded_assets.erase(handle);
  }
}

void AssetManager::handle_pending_loads() {
  ZoneScoped;
  while (p_pending_load_tasks.size() <= p_max_async_tasks_in_flight &&
         !p_queued_loads.empty()) {
    auto &info = p_queued_loads.front();
    dispatch_asset_load_task(info.to_handle(), info);
    p_queued_loads.erase(p_queued_loads.begin());
  }
}

void AssetManager::handle_async_tasks() {
  ZoneScoped;
  std::vector<AssetHandle> finished;
  for (auto &[handle, future] : p_pending_load_tasks) {
    if (Utils::is_future_ready(future)) {
      finished.push_back(handle);
    }
  }

  for (auto &handle : finished) {
    AssetLoadResult asyncReturn = p_pending_load_tasks[handle].get();
    // enqueue new loads
    for (auto &newLoad : asyncReturn.m_new_assets_to_load) {
      load_asset(newLoad.m_path, newLoad.m_type);
    }

    if (asyncReturn.m_asset_load_sync_callbacks.empty() &&
        asyncReturn.m_loaded_asset_intermediate == nullptr) {
      // p_loaded_assets.emplace(handle,
      // std::move(std::unique_ptr<asset>(asyncReturn.m_loaded_asset_intermediate->m_asset_data)));
      transition_asset_to_loaded(
          handle, asyncReturn.m_loaded_asset_intermediate->m_asset_data);
      delete asyncReturn.m_loaded_asset_intermediate;
      asyncReturn.m_loaded_asset_intermediate = nullptr;
    } else {
      p_pending_load_callbacks.emplace(handle, asyncReturn);
    }

    p_pending_load_tasks.erase(handle);
  }
}

void submit_meshes_to_gpu(AssetIntermediate *model_asset) {
  ZoneScoped;
  // cast to model asset
  model_intermediate_asset *inter =
      static_cast<model_intermediate_asset *>(model_asset);
  TAsset<Model, AssetType::model> *ma = inter->get_concrete_asset();
  // go through each entry
  Model::MeshEntry &entry = inter->m_intermediate.front();
  VAOBuilder mesh_builder{};
  mesh_builder.begin();
  std::vector<float> buffer;
  for (int i = 0; i < entry.m_positions.size(); i++) {
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
  for (int i = 0; i < entry.m_indices.size(); i++) {
    indices.push_back(entry.m_indices[i]);
  }

  mesh_builder.add_index_buffer(indices);
  Mesh m{};
  m.m_index_count = static_cast<u32>(indices.size());
  m.m_material_index = entry.m_material_index;
  m.m_original_aabb = entry.m_mesh_aabb;
  m.m_vao = mesh_builder.build();
  // create real mesh object on gpu

  ma->m_data.m_meshes.push_back(m);
  inter->m_intermediate.erase(inter->m_intermediate.begin());
}

void submit_texture_to_gpu(AssetIntermediate *texture_asset) {
  ZoneScoped;
  // cast to model asset
  texture_intermediate_asset *ta_inter =
      static_cast<texture_intermediate_asset *>(texture_asset);
  TAsset<Texture, AssetType::texture> *ta = ta_inter->get_concrete_asset();

  ta->m_data.submit_to_gpu();
  ta_inter->m_intermediate.clear();
}

void link_shader_program(AssetIntermediate *shader_asset) {
  ZoneScoped;
  shader_intermediate_asset *shader_inter =
      static_cast<shader_intermediate_asset *>(shader_asset);

  shader_inter->get_concrete_asset()->m_data =
      GLShader::create_from_composite(shader_inter->m_intermediate);
}

AssetLoadResult load_model_asset_manager(const std::string &path) {
  ZoneScoped;
  std::vector<TextureEntry> associated_textures;
  std::vector<Model::MeshEntry> mesh_entries;
  // move this into a heap allocated object
  Model m = Model::load_model_from_path_entries(path, associated_textures,
                                                mesh_entries);
  TAsset<Model, AssetType::model> *model_asset =
      new TAsset<Model, AssetType::model>(m, path);
  model_intermediate_asset *model_intermediate =
      new model_intermediate_asset(model_asset, mesh_entries, path);
  std::string directory = Utils::get_directory_from_path(path);

  AssetLoadResult ret{};
  for (auto &tex : associated_textures) {
    ret.m_new_assets_to_load.push_back(
        AssetLoadInfo{tex.m_path, AssetType::texture});
  }
  for (int i = 0; i < mesh_entries.size(); i++) {
    ret.m_asset_load_sync_callbacks.push_back(submit_meshes_to_gpu);
  }
  model_intermediate->m_asset_data = model_asset;
  ret.m_loaded_asset_intermediate = model_intermediate;
  return ret;
}

void unload_model_asset_manager(Asset *_asset) {
  ZoneScoped;
  TAsset<Model, AssetType::model> *ma =
      static_cast<TAsset<Model, AssetType::model> *>(_asset);
  ma->m_data.release();
}

AssetLoadResult load_texture_asset_manager(const std::string &path) {
  ZoneScoped;
  AssetLoadResult ret{};
  ret.m_new_assets_to_load = {};
  ret.m_asset_load_sync_callbacks.push_back(submit_texture_to_gpu);

  Texture t{};
  std::vector<unsigned char> binary = Utils::load_binary_from_path(path);

  if (path.find("dds") != std::string::npos) {
    t.load_texture_gli(binary);
  } else {
    t.load_texture_stbi(binary);
  }

  TAsset<Texture, AssetType::texture> *ta =
      new TAsset<Texture, AssetType::texture>(t, path);
  texture_intermediate_asset *ta_inter =
      new texture_intermediate_asset(ta, binary, path);

  ret.m_loaded_asset_intermediate = ta_inter;
  return ret;
}

void unload_texture_asset_manager(Asset *_asset) {
  ZoneScoped;
  TAsset<Texture, AssetType::texture> *ta =
      static_cast<TAsset<Texture, AssetType::texture> *>(_asset);
  ta->m_data.release();
}

AssetLoadResult load_shader_asset_manager(const std::string &path) {
  ZoneScoped;
  std::string source = Utils::load_string_from_path(path);
  AssetLoadResult ret{};
  ret.m_loaded_asset_intermediate = new shader_intermediate_asset(
      new TAsset<GLShader, AssetType::shader>(GLShader{}, path), source,
      path);
  ret.m_asset_load_sync_callbacks.push_back(link_shader_program);
  ret.m_new_assets_to_load = {};

  return ret;
}

void unload_shader_asset_manager(Asset *_asset) {
  ZoneScoped;
  TAsset<GLShader, AssetType::shader> *sa =
      static_cast<TAsset<GLShader, AssetType::shader> *>(_asset);
  sa->m_data.release();
}

void AssetManager::dispatch_asset_load_task(const AssetHandle &handle,
                                             AssetLoadInfo &info) {
  ZoneScoped;
  switch (info.m_type) {
  case AssetType::model:
    p_pending_load_tasks.emplace(
        handle, std::move(std::async(std::launch::async,
                                     load_model_asset_manager, info.m_path)));
    break;
  case AssetType::texture:
    p_pending_load_tasks.emplace(
        handle, std::move(std::async(std::launch::async,
                                     load_texture_asset_manager, info.m_path)));
    break;
  case AssetType::shader:
    p_pending_load_tasks.emplace(
        handle, std::move(std::async(std::launch::async,
                                     load_shader_asset_manager, info.m_path)));
    break;
  }
}

void AssetManager::transition_asset_to_loaded(const AssetHandle &handle,
                                               Asset *asset_to_transition) {
  ZoneScoped;
  p_loaded_assets.emplace(
      handle, std::move(std::unique_ptr<Asset>(asset_to_transition)));
  spdlog::info("asset_manager : loaded {} : {} ",
               get_asset_type_name(handle.m_type), asset_to_transition->m_path);

  if (p_asset_loaded_callbacks.find(handle) == p_asset_loaded_callbacks.end()) {
    return;
  }

  for (auto &[ah, loaded_callback] : p_asset_loaded_callbacks) {
    loaded_callback(p_loaded_assets[ah].get());
  }

  p_asset_loaded_callbacks.erase(handle);
}

AssetHandle AssetLoadInfo::to_handle() {
  ZoneScoped;
  return AssetHandle(m_path, m_type);
}

void AssetManager::unload_asset(const AssetHandle &handle) {
  ZoneScoped;
  if (p_loaded_assets.find(handle) == p_loaded_assets.end()) {
    return;
  }

  switch (handle.m_type) {
  case AssetType::model:
    p_pending_unload_callbacks.emplace(handle, unload_model_asset_manager);
    break;
  case AssetType::texture:
    p_pending_unload_callbacks.emplace(handle, unload_texture_asset_manager);
    break;
  case AssetType::shader:
    p_pending_unload_callbacks.emplace(handle, unload_shader_asset_manager);
    break;
  default:
    break;
  }
}
AssetManager::AssetManager() {
  p_file_watcher = std::make_unique<efsw::FileWatcher>();
  p_gem_listener = std::make_unique<GemFileWatchListener>();
  p_gem_listener->m_watch_id =
      p_file_watcher->addWatch("assets", p_gem_listener.get(), true);
  p_file_watcher->watch();
}
void AssetManager::on_imgui() {
  if (ImGui::Begin("Assets Debug"))
  {
    if (ImGui::CollapsingHeader("CPU Memory")) {
      ImGui::Text("Untracked : %.4f KB", (float)DebugMemoryTracker::s_UntrackedSize / 1024.0f);
      for (auto& [k, v] :
           DebugMemoryTracker::s_instance->s_allocation_info) {
        ImGui::Text("%s : %zu KB", k.c_str(), (v.size * v.count) / 1024);
      }
    }
    ImGui::Separator();
    ImGui::Text("Any Assets Loading? : %s", any_assets_loading() ? "true" : "false");
    ImGui::Text("Any Pending Async Tasks : %d", static_cast<uint32_t>(p_pending_load_tasks.size()));
    ImGui::Text("Any Pending Synchronous Callbacks : %d", static_cast<uint32_t>(p_pending_load_callbacks.size()));
    ImGui::Text("Any Pending Unload Tasks: %d", static_cast<uint32_t>(p_pending_unload_callbacks.size()));

    if (ImGui::CollapsingHeader("Loaded Assets"))
    {
      for (const auto& [handle, u_asset] : p_loaded_assets)
      {
        if (!u_asset) { continue; }
        ImGui::PushID(handle.m_path_hash);
        ImGui::Text("%s : %s", u_asset->m_path.c_str(), get_asset_type_name(handle.m_type).c_str());
        ImGui::SameLine();
        if (ImGui::Button("Unload"))
        {
          unload_asset(handle);
        }
        ImGui::PopID();
      }
    }

    if (ImGui::CollapsingHeader("Enqueued Loads"))
    {
      for (const auto& info : p_queued_loads)
      {
        ImGui::Text("%s : %s", info.m_path.c_str(), get_asset_type_name(info.m_type).c_str());
      }
    }

    if (ImGui::CollapsingHeader("In Progress Loads"))
    {
      for (const auto& [handle , info ]: p_pending_load_callbacks)
      {
        ImGui::Text("%s : %s", info.m_loaded_asset_intermediate->m_asset_data->m_path.c_str(), get_asset_type_name(handle.m_type).c_str());
      }
    }
    ImGui::Separator();
    if (ImGui::Button("Unload All Assets"))
    {
      unload_all_assets();
    }

    if (ImGui::Button("Load Model"))
    {
      ifd::FileDialog::Instance().Open("ModelOpenDialog", "Import a model", "Model file (*.dae;*.obj;*.fbx;*.gltf;){.dae,.obj,.fbx,.gltf},.*");
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Shader"))
    {
      ifd::FileDialog::Instance().Open("ShaderOpenDialog", "Import a shader", "Shader file (*.shader){.shader,.*}");
    }

  }

  if (ifd::FileDialog::Instance().IsDone("ModelOpenDialog")) {
    if (ifd::FileDialog::Instance().HasResult()) {
      std::filesystem::path p = ifd::FileDialog::Instance().GetResult();
      std::string res = p.u8string();
      printf("OPEN[%s]\n", res.c_str());
      load_asset(res, AssetType::model);
    }
    ifd::FileDialog::Instance().Close();
  }

  if (ifd::FileDialog::Instance().IsDone("ShaderOpenDialog")) {
    if (ifd::FileDialog::Instance().HasResult()) {
      std::filesystem::path p = ifd::FileDialog::Instance().GetResult();
      std::string res = p.u8string();
      printf("OPEN[%s]\n", res.c_str());
      load_asset(res, AssetType::shader);
    }
    ifd::FileDialog::Instance().Close();
  }
  ImGui::End();
}
} // namespace gem