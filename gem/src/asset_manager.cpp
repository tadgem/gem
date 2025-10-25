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
                         AssetType::kTexture>;
using model_intermediate_asset =
    TAssetIntermediate<Model, std::vector<Model::MeshEntry>,
                         AssetType::kModel>;
using shader_intermediate_asset =
    TAssetIntermediate<GLShader, std::string, AssetType::kShader>;

AssetHandle AssetManager::LoadAsset(const std::string &path,
                                       const AssetType &assetType,
                                      AssetLoadedCallback on_asset_loaded) {
  ZoneScoped;
  if (!std::filesystem::exists(path)) {
    return {};
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

  auto it = std::find(enqueued_loads_.begin(), enqueued_loads_.end(), load_info);

  for (auto &queued_load : enqueued_loads_) {
    if (load_info == queued_load) {
      return queued_load.to_handle();
    }
  }

  enqueued_loads_.push_back(load_info);

  if (on_asset_loaded != nullptr) {
    asset_loaded_callbacks_.emplace(handle, on_asset_loaded);
  }

  return handle;
}

Asset *AssetManager::GetAsset(AssetHandle &handle) {
  ZoneScoped;
  if (loaded_assets_.find(handle) == loaded_assets_.end()) {
    return nullptr;
  }

  return loaded_assets_[handle].get();
}

AssetLoadProgress
AssetManager::GetLoadProgress(const AssetHandle &handle) {
  ZoneScoped;
  for (auto &queued : enqueued_loads_) {
    if (queued.to_handle() == handle) {
      return AssetLoadProgress::loading;
    }
  }

  if (pending_async_load_tasks_.find(handle) != pending_async_load_tasks_.end() ||
      pending_sync_load_tasks_.find(handle) != pending_sync_load_tasks_.end()) {
    return AssetLoadProgress::loading;
  }

  if (pending_unload_tasks_.find(handle) !=
      pending_unload_tasks_.end()) {
    return AssetLoadProgress::unloading;
  }

  if (loaded_assets_.find(handle) != loaded_assets_.end()) {
    return AssetLoadProgress::loaded;
  }

  return AssetLoadProgress::not_loaded;
}

bool AssetManager::AnyAssetsLoading() {
  ZoneScoped;
  return !pending_async_load_tasks_.empty() || !pending_sync_load_tasks_.empty() ||
         !pending_unload_tasks_.empty() || !enqueued_loads_.empty();
}

bool AssetManager::AnyAssetsUnloading() {
  ZoneScoped;
  return !pending_unload_tasks_.empty();
}

void AssetManager::WaitAllLoads() {
  ZoneScoped;
  while (AnyAssetsLoading()) {
    Update();
  }
}

void AssetManager::WaitAllUnloads() {
  ZoneScoped;
  while (AnyAssetsUnloading()) {
    Update();
  }
}

void AssetManager::UnloadAll() {
  ZoneScoped;
  std::vector<AssetHandle> assetsRemaining{};

  for (auto &[handle, asset] : loaded_assets_) {
    assetsRemaining.push_back(handle);
  }

  for (auto &handle : assetsRemaining) {
    UnloadAsset(handle);
  }

  WaitAllUnloads();
}

void AssetManager::Update() {
  ZoneScoped;
  if (!AnyAssetsLoading()) {
    return;
  }

  DispatchAssetTasksInternal();
  HandlePendingLoadTasksInternal();
  HandlePendingAsyncTasksInternal();
}

void AssetManager::Shutdown() {
  ZoneScoped;
  WaitAllLoads();
  WaitAllUnloads();
  UnloadAll();
}

void AssetManager::DispatchAssetTasksInternal() {
  ZoneScoped;
  u16 processedCallbacks = 0;
  std::vector<AssetHandle> clears;

  for (auto &[handle, asset] : pending_sync_load_tasks_) {
    if (processedCallbacks == kSyncTasksPerFrame)
      break;

    for (u16 i = 0; i < kSyncTasksPerFrame - processedCallbacks; i++) {
      if (i >= asset.sync_load_tasks.size())
        break;
      asset.sync_load_tasks.back()(
          asset.loaded_asset_intermediate);
      asset.sync_load_tasks.pop_back();
      processedCallbacks++;
    }

    if (asset.sync_load_tasks.empty()) {
      clears.push_back(handle);
    }
  }
  for (auto &handle : clears) {
    // p_loaded_assets.emplace(handle,
    // std::move(std::unique_ptr<asset>(p_pending_load_callbacks[handle].m_loaded_asset_intermediate->m_asset_data)));
    FinalizeAssetLoad(handle,
                               pending_sync_load_tasks_[handle]
                                   .loaded_asset_intermediate->asset_data);
    delete pending_sync_load_tasks_[handle].loaded_asset_intermediate;
    pending_sync_load_tasks_.erase(handle);
  }
  clears.clear();

  for (auto &[handle, callback] : pending_unload_tasks_) {
    if (processedCallbacks == kSyncTasksPerFrame)
      break;
    callback(loaded_assets_[handle].get());
    clears.push_back(handle);
    processedCallbacks++;
  }

  for (auto &handle : clears) {
    pending_unload_tasks_.erase(handle);
    loaded_assets_[handle].reset();
    loaded_assets_.erase(handle);
  }
}

void AssetManager::HandlePendingLoadTasksInternal() {
  ZoneScoped;
  while (pending_async_load_tasks_.size() <= kMaxAsyncTasksInFlight &&
         !enqueued_loads_.empty()) {
    auto &info = enqueued_loads_.front();
    DispatchLoadTask(info.to_handle(), info);
    enqueued_loads_.erase(enqueued_loads_.begin());
  }
}

void AssetManager::HandlePendingAsyncTasksInternal() {
  ZoneScoped;
  std::vector<AssetHandle> finished;
  for (auto &[handle, future] : pending_async_load_tasks_) {
    if (Utils::IsFutureReady(future)) {
      finished.push_back(handle);
    }
  }

  for (auto &handle : finished) {
    AssetLoadResult asyncReturn = pending_async_load_tasks_[handle].get();
    // enqueue new loads
    for (auto &newLoad : asyncReturn.new_assets_to_load) {
      LoadAsset(newLoad.path, newLoad.asset_type);
    }

    if (asyncReturn.sync_load_tasks.empty() &&
        asyncReturn.loaded_asset_intermediate == nullptr) {
      // p_loaded_assets.emplace(handle,
      // std::move(std::unique_ptr<asset>(asyncReturn.m_loaded_asset_intermediate->m_asset_data)));
      FinalizeAssetLoad(
          handle, asyncReturn.loaded_asset_intermediate->asset_data);
      delete asyncReturn.loaded_asset_intermediate;
      asyncReturn.loaded_asset_intermediate = nullptr;
    } else {
      pending_sync_load_tasks_.emplace(handle, asyncReturn);
    }

    pending_async_load_tasks_.erase(handle);
  }
}

void submit_meshes_to_gpu(AssetIntermediate *model_asset) {
  ZoneScoped;
  // cast to model asset
  model_intermediate_asset *inter =
      static_cast<model_intermediate_asset *>(model_asset);
  TAsset<Model, AssetType::kModel> *ma = inter->get_concrete_asset();
  // go through each entry
  Model::MeshEntry &entry = inter->intermediate.front();
  VAOBuilder mesh_builder{};
  mesh_builder.Begin();
  std::vector<float> buffer;
  for (int i = 0; i < entry.positions.size(); i++) {
    buffer.push_back(entry.positions[i].x);
    buffer.push_back(entry.positions[i].y);
    buffer.push_back(entry.positions[i].z);
    buffer.push_back(entry.normals[i].x);
    buffer.push_back(entry.normals[i].y);
    buffer.push_back(entry.normals[i].z);
    buffer.push_back(entry.uvs[i].x);
    buffer.push_back(entry.uvs[i].y);
  }

  mesh_builder.AddVertexBuffer(buffer);
  mesh_builder.AddVertexAttribute(0, 8 * sizeof(float), 3);
  mesh_builder.AddVertexAttribute(1, 8 * sizeof(float), 3);
  mesh_builder.AddVertexAttribute(2, 8 * sizeof(float), 2);

  std::vector<u32> indices;
  for (int i = 0; i < entry.indices.size(); i++) {
    indices.push_back(entry.indices[i]);
  }

  mesh_builder.AddIndexBuffer(indices);
  AMesh* m = new AMesh();
  m->index_count = static_cast<u32>(indices.size());
  m->material_index = entry.material_index;
  m->original_aabb = entry.mesh_aabb;
  m->vao = mesh_builder.BuildVAO();
  // create real mesh object on gpu

  ma->data.meshes.push_back(m);
  inter->intermediate.erase(inter->intermediate.begin());
}

void submit_texture_to_gpu(AssetIntermediate *texture_asset) {
  ZoneScoped;
  // cast to model asset
  texture_intermediate_asset *ta_inter =
      static_cast<texture_intermediate_asset *>(texture_asset);
  TAsset<Texture, AssetType::kTexture> *ta = ta_inter->get_concrete_asset();

  ta->data.SubmitToGPU();
  ta_inter->intermediate.clear();
}

void link_shader_program(AssetIntermediate *shader_asset) {
  ZoneScoped;
  shader_intermediate_asset *shader_inter =
      static_cast<shader_intermediate_asset *>(shader_asset);

  shader_inter->get_concrete_asset()->data =
      GLShader::CreateFromComposite(shader_inter->intermediate);
}

AssetLoadResult load_model_asset_manager(const std::string &path) {
  ZoneScoped;
  std::vector<TextureEntry> associated_textures;
  std::vector<Model::MeshEntry> mesh_entries;
  // move this into a heap allocated object
  Model m = Model::LoadModelAndTextureEntries(path, associated_textures,
                                                mesh_entries);
  TAsset<Model, AssetType::kModel> *model_asset =
      new TAsset<Model, AssetType::kModel>(m, path);
  model_intermediate_asset *model_intermediate =
      new model_intermediate_asset(model_asset, mesh_entries, path);
  std::string directory = Utils::GetDirFromPath(path);

  AssetLoadResult ret{};
  for (auto &tex : associated_textures) {
    ret.new_assets_to_load.push_back(
        AssetLoadInfo{tex.path, AssetType::kTexture});
  }
  for (int i = 0; i < mesh_entries.size(); i++) {
    ret.sync_load_tasks.push_back(submit_meshes_to_gpu);
  }
  model_intermediate->asset_data = model_asset;
  ret.loaded_asset_intermediate = model_intermediate;
  return ret;
}

void unload_model_asset_manager(Asset *_asset) {
  ZoneScoped;
  TAsset<Model, AssetType::kModel> *ma =
      static_cast<TAsset<Model, AssetType::kModel> *>(_asset);
  ma->data.Release();
}

AssetLoadResult load_texture_asset_manager(const std::string &path) {
  ZoneScoped;
  AssetLoadResult ret{};
  ret.new_assets_to_load = {};
  ret.sync_load_tasks.push_back(submit_texture_to_gpu);

  Texture t{};
  std::vector<unsigned char> binary = Utils::LoadBinaryFromPath(path);

  if (path.find("dds") != std::string::npos) {
    t.LoadTextureGLI(binary);
  } else {
    t.LoadTextureSTB(binary);
  }

  TAsset<Texture, AssetType::kTexture> *ta =
      new TAsset<Texture, AssetType::kTexture>(t, path);
  texture_intermediate_asset *ta_inter =
      new texture_intermediate_asset(ta, binary, path);

  ret.loaded_asset_intermediate = ta_inter;
  return ret;
}

void unload_texture_asset_manager(Asset *_asset) {
  ZoneScoped;
  TAsset<Texture, AssetType::kTexture> *ta =
      static_cast<TAsset<Texture, AssetType::kTexture> *>(_asset);
  ta->data.ReleaseGPU();
}

AssetLoadResult load_shader_asset_manager(const std::string &path) {
  ZoneScoped;
  std::string source = Utils::LoadStringFromPath(path);
  AssetLoadResult ret{};
  ret.loaded_asset_intermediate = new shader_intermediate_asset(
      new TAsset<GLShader, AssetType::kShader>(GLShader{}, path), source,
      path);
  ret.sync_load_tasks.push_back(link_shader_program);
  ret.new_assets_to_load = {};

  return ret;
}

void unload_shader_asset_manager(Asset *_asset) {
  ZoneScoped;
  TAsset<GLShader, AssetType::kShader> *sa =
      static_cast<TAsset<GLShader, AssetType::kShader> *>(_asset);
  sa->data.Release();
}

void AssetManager::DispatchLoadTask(const AssetHandle &handle,
                                             AssetLoadInfo &info) {
  ZoneScoped;
  switch (info.asset_type) {
  case AssetType::kModel:
    pending_async_load_tasks_.emplace(
        handle, std::move(std::async(std::launch::async,
                                     load_model_asset_manager, info.path)));
    break;
  case AssetType::kTexture:
    pending_async_load_tasks_.emplace(
        handle, std::move(std::async(std::launch::async,
                                     load_texture_asset_manager, info.path)));
    break;
  case AssetType::kShader:
    pending_async_load_tasks_.emplace(
        handle, std::move(std::async(std::launch::async,
                                     load_shader_asset_manager, info.path)));
    break;
  }
}

void AssetManager::FinalizeAssetLoad(const AssetHandle &handle,
                                               Asset *asset_to_transition) {
  ZoneScoped;
  loaded_assets_.emplace(
      handle, std::move(std::unique_ptr<Asset>(asset_to_transition)));
  spdlog::info("asset_manager : loaded {} : {} ",
               get_asset_type_name(handle.asset_type), asset_to_transition->path);

  if (asset_loaded_callbacks_.find(handle) == asset_loaded_callbacks_.end()) {
    return;
  }

  for (auto &[ah, loaded_callback] : asset_loaded_callbacks_) {
    loaded_callback(loaded_assets_[ah].get());
  }

  asset_loaded_callbacks_.erase(handle);
}

AssetHandle AssetLoadInfo::to_handle() {
  ZoneScoped;
  return AssetHandle(path, asset_type);
}

void AssetManager::UnloadAsset(const AssetHandle &handle) {
  ZoneScoped;
  if (loaded_assets_.find(handle) == loaded_assets_.end()) {
    return;
  }

  switch (handle.asset_type) {
  case AssetType::kModel:
    pending_unload_tasks_.emplace(handle, unload_model_asset_manager);
    break;
  case AssetType::kTexture:
    pending_unload_tasks_.emplace(handle, unload_texture_asset_manager);
    break;
  case AssetType::kShader:
    pending_unload_tasks_.emplace(handle, unload_shader_asset_manager);
    break;
  default:
    break;
  }
}
AssetManager::AssetManager() {
  file_watcher_ = std::make_unique<efsw::FileWatcher>();
  gem_listener_ = std::make_unique<GemFileWatchListener>();
  gem_listener_->watch_id =
      file_watcher_->addWatch("assets", gem_listener_.get(), true);
  file_watcher_->watch();
}
void AssetManager::OnImGui() {
  if (ImGui::Begin("Assets Debug"))
  {
    if (ImGui::CollapsingHeader("CPU Memory")) {
      ImGui::Text("Untracked : %.4f KB", (float)DebugMemoryTracker::kUntrackedSize / 1024.0f);
      for (auto& [k, v] :
           DebugMemoryTracker::kInstance->allocation_info) {
        ImGui::Text("%s : %zu KB", k.c_str(), (v.size * v.count) / 1024);
      }
    }
    ImGui::Separator();
    ImGui::Text("Any Assets Loading? : %s", AnyAssetsLoading() ? "true" : "false");
    ImGui::Text("Any Pending Async Tasks : %d", static_cast<uint32_t>(pending_async_load_tasks_.size()));
    ImGui::Text("Any Pending Synchronous Callbacks : %d", static_cast<uint32_t>(pending_sync_load_tasks_.size()));
    ImGui::Text("Any Pending Unload Tasks: %d", static_cast<uint32_t>(pending_unload_tasks_.size()));

    if (ImGui::CollapsingHeader("Loaded Assets"))
    {
      for (const auto& [handle, u_asset] : loaded_assets_)
      {
        if (!u_asset) { continue; }
        ImGui::PushID(handle.path_hash);
        ImGui::Text("%s : %s", u_asset->path.c_str(), get_asset_type_name(handle.asset_type).c_str());
        ImGui::SameLine();
        if (ImGui::Button("Unload"))
        {
          UnloadAsset(handle);
        }
        ImGui::PopID();
      }
    }

    if (ImGui::CollapsingHeader("Enqueued Loads"))
    {
      for (const auto& info : enqueued_loads_)
      {
        ImGui::Text("%s : %s", info.path.c_str(), get_asset_type_name(info.asset_type).c_str());
      }
    }

    if (ImGui::CollapsingHeader("In Progress Loads"))
    {
      for (const auto& [handle , info ]: pending_sync_load_tasks_)
      {
        ImGui::Text("%s : %s", info.loaded_asset_intermediate->asset_data->path.c_str(), get_asset_type_name(handle.asset_type).c_str());
      }
    }
    ImGui::Separator();
    if (ImGui::Button("Unload All Assets"))
    {
      UnloadAll();
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
      LoadAsset(res, AssetType::kModel);
    }
    ifd::FileDialog::Instance().Close();
  }

  if (ifd::FileDialog::Instance().IsDone("ShaderOpenDialog")) {
    if (ifd::FileDialog::Instance().HasResult()) {
      std::filesystem::path p = ifd::FileDialog::Instance().GetResult();
      std::string res = p.u8string();
      printf("OPEN[%s]\n", res.c_str());
      LoadAsset(res, AssetType::kShader);
    }
    ifd::FileDialog::Instance().Close();
  }
  ImGui::End();
}
} // namespace gem