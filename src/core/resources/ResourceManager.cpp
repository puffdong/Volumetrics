#include "ResourceManager.hpp"
#include <optional>
#include <string>
#include <memory>
#include <iostream>

#include "core/rendering/Shader.hpp"
#include "core/rendering/Model.hpp"
#include "core/rendering/Texture.hpp"

#include "core/resources/adapters/ModelAdapter.hpp"

ResourceManager::ResourceManager(const std::string& assets_root_path, const std::string& assets_handle)
: root_path(assets_root_path), asset_handle(assets_handle)
{

}

std::string ResourceManager::get_full_path(const std::string& asset_path) {
    return std::string(root_path) + std::string(asset_path.substr(asset_handle.size(), asset_path.size()));
}

ResourceID ResourceManager::generate_new_id() {
    return UUID<Resource>{};
}

Resource ResourceManager::load_shader(const std::string& shader_asset_path) {
    ResourceID resource_id = generate_new_id();
    shader_map[resource_id] = std::make_unique<Shader>(get_full_path(shader_asset_path));

    Resource s_res = { resource_id, shader_asset_path, ResourceType::Shader };
    resource_map[resource_id] = std::move(s_res); // keep resource thing in here

    return s_res;
}

Resource ResourceManager::load_shader(const std::string& vertex_asset_path, const std::string& fragment_asset_path) {
    ResourceID resource_id = generate_new_id();
    shader_map[resource_id] = std::make_unique<Shader>(get_full_path(vertex_asset_path), get_full_path(fragment_asset_path)); 

    // didn't account for the possibility of having multiple paths for what is semantically one resource, eeehhhh....
    Resource s_res = { resource_id, vertex_asset_path, ResourceType::Shader }; // let's just save the vertex_path for now xd
    resource_map[resource_id] = std::move(s_res);

    return s_res;
}


std::optional<Shader*> ResourceManager::get_shader(ResourceID resource_id) {
    if (resource_map.find(resource_id) != resource_map.end()) {
        Resource res = resource_map[resource_id];
        if (res.type == ResourceType::Shader) { 
            return shader_map[res.id].get(); 
        } else {
            return std::nullopt;
        }
    } else {
        return std::nullopt;
    }
}

Res::Model ResourceManager::load_model(const std::string& asset_path) {
    const std::string file_path = get_full_path(asset_path);
    
    ModelResource model;
    model.id = UUID<Res::Model>();
    model.name = std::filesystem::path(file_path).filename().string();
    model.asset_path = asset_path;
    model.file_path = file_path;
    
    model.gpu_data = ModelAdapter::load_obj(file_path);

    Res::Model r_model;
    r_model.id = model.id;
    r_model.name = model.name;
    r_model.asset_path = model.asset_path;

    model_map[r_model.id] = std::move(model);

    return r_model;
}

Res::Model ResourceManager::upload_model(ModelGpuData data) {
    ModelResource model_res;
    model_res.id = UUID<Res::Model>();
    model_res.name = data.name;
    model_res.asset_path = "gen://create_flat_ground()";
    model_res.file_path = "gen://create_flat_ground()";

    model_res.gpu_data = std::move(data);

    Res::Model r_model;
    r_model.id = model_res.id;
    r_model.name = model_res.name;
    r_model.asset_path = model_res.asset_path;

    model_map[r_model.id] = std::move(model_res);

    return r_model;

}

ModelGpuData ResourceManager::get_model_gpu_data(const ModelID res_id) {
    return model_map[res_id].gpu_data;
}