#include "ResourceManager.hpp"
#include <optional>
#include <string>
#include <iostream>

#include "core/rendering/Shader.hpp"
#include "core/rendering/Model.hpp"
#include "core/rendering/Texture.hpp"

#include "core/resources/adapters/ModelAdapter.hpp"

ResourceManager::ResourceManager(const std::string& assets_root_path, const std::string& assets_handle)
: root_path(assets_root_path), asset_handle(assets_handle)
{}

std::string ResourceManager::get_full_path(const std::string& asset_path) {
    return std::string(root_path) + std::string(asset_path.substr(asset_handle.size(), asset_path.size()));
}

Res::Model ResourceManager::load_model(const std::string& asset_path) {
    const std::string file_path = get_full_path(asset_path);
    
    if (_model_path_to_model_id_cache.find(file_path) != _model_path_to_model_id_cache.end()) {
        ModelID existing_id = _model_path_to_model_id_cache[file_path];
        std::cout << "Model already loaded, returning existing resource. Path: " << asset_path << std::endl;
        return { existing_id, _model_map[existing_id].name, _model_map[existing_id].file_extension };
    }
    
    std::string file_extension = std::filesystem::path(file_path).extension().string();

    ModelResource model;
    model.id = UUID<Res::Model>();
    model.name = std::filesystem::path(file_path).filename().string();
    model.asset_path = asset_path;
    model.file_path = file_path;
    model.file_extension = file_extension;
    
    if (file_extension == ".obj") {
        model.gpu_data = ModelAdapter::load_obj(file_path);
    } else if (file_extension == ".gltf" || file_extension == ".glb") {
        model.gpu_data_2 = ModelAdapter::load_gltf(file_path);
    } else {
        std::cerr << "Unsupported model format: " << file_extension << std::endl;
        // Handle unsupported format, maybe throw an exception or return an error code
    }

    Res::Model r_model;
    r_model.id = model.id;
    r_model.name = model.name;
    r_model.file_extension = model.file_extension;

    _model_map[r_model.id] = std::move(model);
    _model_path_to_model_id_cache[file_path] = r_model.id;

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
    r_model.file_extension = "generated";

    _model_map[r_model.id] = std::move(model_res);

    return r_model;

}

const ModelGpuData& ResourceManager::get_model_gpu_data(const Res::Model& resource) {
    return _model_map[resource.id].gpu_data;
}

const ModelGpuData2& ResourceManager::get_model_gpu_data_2(const Res::Model& resource) {
    return _model_map[resource.id].gpu_data_2;
}