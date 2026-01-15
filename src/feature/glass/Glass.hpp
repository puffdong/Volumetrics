#include "glm/glm.hpp"
#include "core/resources/ResourceManager.hpp"
#include "core/rendering/Renderer.hpp"
#include "core/utils/ButtonMap.hpp"

class Glass {
private:
    glm::vec2 position;
    Resource r_shader;
    bool _visible = true;

public: 
    Glass() = default;

    void init(ResourceManager& resources);
    void tick(float delta, const ButtonMap& button_map);
    void enqueue(Renderer& renderer, ResourceManager& resources, const ButtonMap& button_map);
};