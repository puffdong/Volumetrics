#pragma once
#include <memory>
#include <optional>
#include <filesystem>
#include <unordered_map>
#include "core/rendering/Shader.hpp"
#include "core/UUID.hpp"
#include "core/rendering/Model.hpp"

namespace Res {
    struct Model {
        ModelID id;
        std::string name;
        std::string asset_path;
    };
}    

struct ModelResource {
    ModelID id;
    std::string name;
    std::string asset_path;
    std::string file_path;

    ModelGpuData gpu_data;
};

using ResourceID = UUID<Resource>;

enum class ResourceType { Shader, Texture, Model, None };

struct Resource {
    ResourceID id;
    std::string asset_path = "";
    ResourceType type = ResourceType::None;
};

class ResourceManager {    
private:
    std::string root_path;
    std::string asset_handle;

    std::unordered_map<ResourceID, Resource, uuid_hash<Resource>> resource_map;
    std::unordered_map<ResourceID, std::unique_ptr<Shader>, uuid_hash<Resource>> shader_map;

    std::unordered_map<ModelID, ModelResource, uuid_hash<Res::Model>> model_map;


public:
    ResourceManager(const std::string& assets_root_path, const std::string& assets_handle = "res://");
    std::string get_full_path(const std::string& asset_path);
    
    Resource load_shader(const std::string& vertex_asset_path, const std::string& fragment_asset_path);
    
    std::optional<Shader*> get_shader(UUID<Resource> resource_id);
    
    Res::Model load_model(const std::string& asset_path);
    Res::Model upload_model(ModelGpuData data);
    ModelGpuData get_model_gpu_data(const ModelID res_id);

private:
    ResourceID generate_new_id();
};