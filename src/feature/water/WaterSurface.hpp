#pragma once
#include "core/Base.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "core/OBJLoader.hpp"


class WaterSurface : public Base {
private:
    Resource r_shader;

    Shader* shader;
    ModelObject* model;

    float height;
    float width;

    float time;
    
public:
    WaterSurface(glm::vec3 pos = glm::vec3(0.f),
                 glm::vec3 rot = glm::vec3(0.f),
                 glm::vec3 scale = glm::vec3(1.f), 
                 Base* parent = nullptr,
                 float height = 1.f, float width = 1.f);
    
    WaterSurface(glm::vec3 pos, float height, float width);

    void init(ResourceManager& resources, Space* space) override;
    void tick(float delta) override;
    void enqueue(Renderer& renderer, ResourceManager& resources) override;

    void render(Renderer& renderer, glm::mat4 proj, glm::mat4 view, glm::vec3 camera_pos);
};