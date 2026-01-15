#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "core/resources/ResourceManager.hpp"
#include "core/rendering/Renderer.hpp"

struct RaymarchSettings {
    int max_steps = 256;
    float step_size = 0.3f; 
    float hit_step_size = 0.15f;
    int max_light_steps = 16;
    float light_step_size = 0.6f;
    float max_distance = 512.f;
    float min_distance = 0.000001f;

    glm::vec3 base_color = glm::vec3(0.05f, 0.05f, 0.05f);
    float absorption_coefficient = 1.0f;
    float scattering_coefficient = 0.6f;
    float extincion_coefficient = 1.6f;

    float anisotropy = 0.5f; // for the HG phase function
    float sun_intensity = 20.f;
};

class Raymarcher {
private:
    Resource r_shader;
    GLuint perlin3d;

    float time = 0.0;
    RaymarchSettings raymarch_settings;

    bool _visible = true;

public:
    Raymarcher();
    void init(ResourceManager& resources);
    void tick(float delta);
    void enqueue(Renderer& renderer, ResourceManager& resources, glm::vec3 camera_pos, glm::vec3 sun_dir, glm::vec3 sun_color, unsigned int voxel_tex, glm::ivec3 grid_dim, glm::vec3 grid_origin, float cell_size);

    void set_visibility(const bool v) { _visible = v; };
    bool is_visible() const { return _visible; };
    RaymarchSettings& get_raymarch_settings() { return raymarch_settings; };

private:
    void upload_uniforms(Renderer& renderer, Shader* shader, glm::vec3 camera_pos, glm::vec3 sun_dir, glm::vec3 sun_color, glm::ivec3 grid_dim, glm::vec3 grid_origin, float cell_size);
};