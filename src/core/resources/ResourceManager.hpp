#pragma once
// OpenGL stuff
#include <GL/glew.h>

// std libs
#include <optional>
#include <filesystem>  
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <optional>

// Core
#include "core/rendering/Shader.hpp"
#include "core/rendering/Model.hpp"
#include "core/rendering/Texture.hpp"


enum ResourceType {
    NONE = -1,
    SHADER = 0,
    TEXTURE = 1,
    MODEL = 2
};

struct Resource {
    int id = -1;
    std::string asset_path = "";
    ResourceType type = NONE;
};

class ResourceManager {    
private:
    std::string root_path;
    std::string asset_handle;

    std::unordered_map<int, Resource> resoruce_map;
    std::unordered_map<int, Shader*> shader_map;

    int number_of_resources = 0; // increments every time a new object is loaded regardless of what it is
    std::unordered_set<int> resource_id_set;

public:
    ResourceManager(const std::string& assets_root_path, const std::string& assets_handle = "res://");
    std::string get_full_path(const std::string& asset_path);
    
    Resource load_shader(const std::string& shader_asset_path);
    // I am going to abstract this thing below later but need it for now heh
    // The plan is to not even have "shaders" be a thing and any uniform and such is going to just be 
    // abstracted away blah blah, we working on it yuh yuh
    std::optional<Shader*> get_shader(int resource_id);

    void unload_all_resources(); // shutdown of everything
    
private:
    int generate_new_id();


};