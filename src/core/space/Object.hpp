#pragma once
#include "core/Base.hpp"

class Space; // fwd decl

class Object : public Base {
private:
    Resource r_shader;
    Res::Model r_model;
    Resource r_texture;

public:
    Object(glm::vec3 pos = glm::vec3(0.f),
           glm::vec3 rot = glm::vec3(0.f),
           glm::vec3 scale = glm::vec3(1.f), 
           const std::string& shader_path = "",
           const std::string& model_path = "",
           const std::string& texture_path = "");
    
    void init(ResourceManager& resources, Space* space) override;
    void tick(float delta) override;
    void enqueue(Renderer& renderer, ResourceManager& resources) override;

    void set_model(Res::Model res);
};