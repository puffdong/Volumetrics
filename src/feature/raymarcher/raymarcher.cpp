#include "raymarcher.hpp"
#include <iostream>
#include "core/rendering/Shader.hpp"

void Raymarcher::init(Shader* shader) {
    _shader = shader;

    perlin_texture = PerlinNoiseTexture();
    init_perlin(perlin_texture, 128, 128, 128, 0.05f,42);
    generate_perlin(perlin_texture);
    upload_perlin(perlin_texture);
}

void Raymarcher::tick(float delta) {
    _time += delta;
}

void Raymarcher::enqueue(Renderer& renderer, glm::vec3 camera_pos, glm::vec3 sun_dir, glm::vec4 sun_color, unsigned int voxel_tex, glm::ivec3 grid_dim, glm::vec3 grid_origin, float cell_size) {
    if (!_visible) return;

    _shader->hot_reload_if_changed();
    _shader->bind();
    upload_uniforms(renderer, camera_pos, sun_dir, sun_color, grid_dim, grid_origin, cell_size);

    TextureBinding perlin_noise{ perlin_texture.texture_id, GL_TEXTURE_3D, 5, "u_noise_texture" };
    TextureBinding vox_tex{ voxel_tex, GL_TEXTURE_3D, 6, "u_voxels" };

    RenderCommand cmd{};
    cmd.draw_type = DrawType::FullscreenQuad;
    cmd.shader = _shader;
    cmd.state.depth_write = false;
    cmd.textures.push_back(perlin_noise);
    cmd.textures.push_back(vox_tex);
    cmd.attach_lights = true;

    renderer.submit(RenderPass::Volumetrics, cmd);
}

void Raymarcher::upload_uniforms(Renderer& renderer, glm::vec3 camera_pos, glm::vec3 sun_dir, glm::vec4 sun_color, glm::ivec3 grid_dim, glm::vec3 grid_origin, float cell_size) {
        glm::mat4 proj = renderer.get_proj();
        glm::mat4 view = renderer.get_view();
        glm::mat4 inverted_proj_view = glm::inverse(proj * view);

        // standard uniforms
        _shader->set_uniform_mat4("u_invprojview", inverted_proj_view);
        _shader->set_uniform_vec3("u_camera_pos", camera_pos);
        _shader->set_uniform_vec3("u_sun_dir", sun_dir);
        _shader->set_uniform_vec4("u_sun_color", sun_color); // .w = intensity
        _shader->set_uniform_float("u_time", _time);
        _shader->set_uniform_vec2("u_resolution", renderer.get_viewport_size());

        // depth textures
        _shader->set_uniform_int("u_scene_depth", 2);
        _shader->set_uniform_int("u_raymarch_depth", 3);

        // voxel grid params
        _shader->set_uniform_ivec3("u_grid_dim", grid_dim);
        _shader->set_uniform_vec3("u_grid_origin", grid_origin);
        _shader->set_uniform_float("u_cell_size", cell_size);

        // raymarch parameters
        _shader->set_uniform_int("u_max_steps", raymarch_settings.max_steps);
        _shader->set_uniform_float("u_step_size", raymarch_settings.step_size);
        _shader->set_uniform_int("u_max_light_steps", raymarch_settings.max_light_steps);
        _shader->set_uniform_float("u_light_step_size", raymarch_settings.light_step_size);
        _shader->set_uniform_vec3("u_base_color", raymarch_settings.base_color);
        _shader->set_uniform_float("u_absorption_coefficient", raymarch_settings.absorption_coefficient);
        _shader->set_uniform_float("u_scattering_coefficient", raymarch_settings.scattering_coefficient);
        _shader->set_uniform_float("u_extincion_coefficient", raymarch_settings.extincion_coefficient);
        _shader->set_uniform_float("u_anisotropy", raymarch_settings.anisotropy);
        _shader->set_uniform_float("u_sun_intensity", raymarch_settings.sun_intensity);
}