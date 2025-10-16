#pragma once
// std libs
#include <optional>
#include <filesystem>  
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory> // unique ptr yuh

// Core
#include "core/rendering/Shader.hpp"
#include "core/rendering/Model.hpp"
#include "core/rendering/Texture.hpp"
#include "core/UUID.hpp"

// todo, make Resource<type> thing someday, would be cool to do

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

    // std::unordered_set<ResourceID> active_resource_ids;
    std::unordered_map<ResourceID, Resource, uuid_hash<Resource>> resource_map;
    std::unordered_map<ResourceID, std::unique_ptr<Shader>, uuid_hash<Resource>> shader_map;

    int number_of_resources = 0;
    std::unordered_set<int> resource_id_set;

public:
    ResourceManager(const std::string& assets_root_path, const std::string& assets_handle = "res://");
    std::string get_full_path(const std::string& asset_path);
    
    Resource load_shader(const std::string& shader_asset_path);
    Resource load_shader(const std::string& vertex_asset_path, const std::string& fragment_asset_path);
    // I am going to abstract this thing below later but need it for now heh
    // The plan is to not even have "shaders" be a thing and any uniform and such is going to just be 
    // abstracted away blah blah, we working on it yuh yuh
    std::optional<Shader*> get_shader(UUID<Resource> resource_id);

    void unload_all_resources(); // shutdown of everything, TODO
    
private:
    ResourceID generate_new_id();


};