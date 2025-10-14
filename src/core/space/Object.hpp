#pragma once
#include "core/Base.hpp"

// to be removed when resource manager is cooler
#include "core/OBJLoader.hpp"

class Space; // fwd decl

class Object : public Base {
private:
    Resource r_shader;
    Resource r_model;
    Resource r_texture;

    // for now before we rework it, I just want to get stuff being a Base thing
    ModelObject* _model;
	Texture* _texture;

public:
    Object(glm::vec3 pos = glm::vec3(0.f),
           glm::vec3 rot = glm::vec3(0.f),
           glm::vec3 scale = glm::vec3(1.f), 
           Base* parent = nullptr,
           const std::string& shader_path = "",
           const std::string& model_path = "",
           const std::string& texture_path = "");
    
    void init(ResourceManager& resources, Space* space) override;
    void tick(float delta) override;
    void enqueue(Renderer& renderer, ResourceManager& resources) override;

    void swap_model(ModelObject* model); // shitty but I need it for a quick thing
};