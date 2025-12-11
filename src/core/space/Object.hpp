#pragma once
#include "core/Base.hpp"

// to be removed when resource manager is cooler
#include "core/OBJLoader.hpp"
#include "core/Line.hpp"

class Space; // fwd decl

// My plan for this class is to just have some sort of way of neatly loading models into the world. 
// I'll try and keep it light-weight
class Object : public Base {
private:
    Resource r_shader;
    Res::Model r_model;
    Resource r_texture;

    Line* collision_lines = nullptr;
    bool draw_collision = false;

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

    void set_model(Res::Model res);

    void set_draw_collision(const bool v) { draw_collision = true; }
    bool get_draw_collision() const { return draw_collision; }
    
private:
    // This is a start to get selectable objects to work somehow. Now we at least get the
    // bounds of something visible
    // its jank, I need to fix parenting with lines, there are a ton of todos
    // eh lemme list my thoughts on this 
    /* Needed stuff:
        - child the lines and have them translate the lines based of parent position
            - need to redo a bit of the shader logic for that to account for that
        - we are loading a fresh instance of the line shader for every object. 
            - get the resourcemanager to just re-use the line shader everytime we want it
            - this entails also making sure the uniforms get managed so we need to change
            the renderer and have it be responsible for the uniform setting
            - good thing i got the shader adapter!
                - we could have it so nothing needs to ever import shaders
                - but thats a bit too much effort for now
        - yeesh
    */
    void init_collision_bounds_debug(ResourceManager& resources, Space* space);
};