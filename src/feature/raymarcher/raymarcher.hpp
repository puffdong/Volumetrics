#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "core/Base.hpp"
#include "feature/raymarcher/VoxelGrid.hpp"

class Space;

struct RaymarchSettings {
    int max_steps = 256;
    float step_size = 0.3f; 
    float hit_step_size = 0.15f;
    int max_light_steps = 16;
    float light_step_size = 0.6f;
    float max_distance = 512.f;
    float min_distance = 0.000001f;

    glm::vec3 base_color = glm::vec3(0.05f, 0.05f, 0.05f);
    float absorption_coefficient = 0.5f;
    float scattering_coefficient = 0.5f;
    float extincion_coefficient = 0.25f;
};

class Raymarcher : public Base {
private:
    Resource r_shader;

    VoxelGrid* voxel_grid;
    GLuint perlin3d;

    float time = 0.0;
    RaymarchSettings raymarch_settings;

public:
    Raymarcher();
    void init(ResourceManager& resources, Space* space) override;
    void tick(float delta) override;
    void enqueue(Renderer& renderer, ResourceManager& resources) override;

private:
    void upload_uniforms(Renderer& renderer, Shader* shader);
};


