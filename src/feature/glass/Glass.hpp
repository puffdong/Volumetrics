#pragma once
#include "glm/glm.hpp"
#include "core/resources/ResourceManager.hpp"
#include "core/rendering/Renderer.hpp"
#include "core/utils/ButtonMap.hpp"

class Glass {
private:
    glm::vec2 position;
    Shader* shader = nullptr;
    bool _visible = true;

public: 
    Glass() = default;

    void init(ResourceManager& resources);
    void tick(float delta, const ButtonMap& button_map);
    void enqueue(Renderer& renderer, const ButtonMap& button_map);
    
    void set_visibility(const bool v) { _visible = v; };
    bool is_visible() const { return _visible; };
};