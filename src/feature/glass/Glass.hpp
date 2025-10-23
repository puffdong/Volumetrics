#include "core/Base.hpp"
#include <vector>

class Space;

struct GlassPane {
    glm::vec2 position;
};

class Glass : public Base {
private:
    Resource r_shader;
    std::vector<GlassPane> glass_panes;

public: 
    Glass();

    void init(ResourceManager& resources, Space* space) override;
    void tick(float delta) override;
    void enqueue(Renderer& renderer, ResourceManager& resources) override;

private:
    
};