#include "core/Base.hpp"

class Object : public Base {
private:
    Resource r_shader;
    Resource r_model;
    Resource r_texture;

public:
    Object(glm::vec3 pos = glm::vec3(0.f),
           glm::vec3 rot = glm::vec3(0.f),
           glm::vec3 scale = glm::vec3(1.f), 
           Base* parent = nullptr,
           const std::string& shader_path = "",
           const std::string& model_path = "",
           const std::string& texture_path = "");
    
    void init(ResourceManager& resources, uint64_t id) override;
    void tick(float delta, ButtonMap bm) override;
    void enqueue(Renderer& renderer, ResourceManager& resources) override;
};