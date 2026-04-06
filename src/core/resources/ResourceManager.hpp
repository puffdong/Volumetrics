#pragma once
#include <memory>
#include <optional>
#include <filesystem>
#include <unordered_map>
#include "Resource.hpp"
#include "core/rendering/Shader.hpp"

class ResourceManager {
private:
    std::string root_path;
    std::string asset_handle;

    std::unordered_map<ModelID, ModelResource, uuid_hash<Res::Model>> _model_map;
    std::unordered_map<std::string, ModelID> _model_path_to_model_id_cache;

public:
    ResourceManager(const std::string& assets_root_path, const std::string& assets_handle = "res://");
    std::string get_full_path(const std::string& asset_path);
    
    Res::Model load_model(const std::string& asset_path);
    Res::Model upload_model(ModelGpuData data);
    
    const ModelGpuData& get_model_gpu_data(const Res::Model& resource); // are references to things in unordered_maps stable?
};