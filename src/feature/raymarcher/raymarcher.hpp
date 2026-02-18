#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "core/rendering/Renderer.hpp"
#include "core/utils/perlin_noise_generator.hpp"

struct RaymarchSettings {
    int max_steps = 128;
    float step_size = 0.75f; 
    int max_light_steps = 8;
    float light_step_size = 1.5f; // previously at 0.6f, higher values seem to work better?

    glm::vec3 base_color = glm::vec3(0.05f, 0.05f, 0.05f);
    float absorption_coefficient = 1.0f;
    float scattering_coefficient = 0.6f;

    float anisotropy = 0.35f; // for the HG phase function
    float sun_intensity_multiplier = 20.f;
};

class Raymarcher {
private:
    Shader* _shader;
    float _time = 0.0;
    bool _visible = true;
    
    RaymarchSettings raymarch_settings;
    PerlinNoiseTexture perlin_texture;
    
    
public:
    Raymarcher() = default;
    void init(Shader* shader);
    void tick(float delta);
    void enqueue(Renderer& renderer, glm::vec3 camera_pos, glm::vec3 sun_dir, glm::vec4 sun_color, unsigned int voxel_tex, glm::ivec3 grid_dim, glm::vec3 grid_origin, float cell_size);

    void set_visibility(const bool v) { _visible = v; };
    bool is_visible() const { return _visible; };
    RaymarchSettings& get_raymarch_settings() { return raymarch_settings; };
    PerlinNoiseTexture& get_perlin_texture() { return perlin_texture; };

private:
    void upload_uniforms(Renderer& renderer, glm::vec3 camera_pos, glm::vec3 sun_dir, glm::vec4 sun_color, glm::ivec3 grid_dim, glm::vec3 grid_origin, float cell_size);
};