#include "raymarcher.hpp"
#include <iostream>
#include "core/utils/perlin_noise_generator.hpp"

void Raymarcher::init(ResourceManager& resources) {
    r_shader = resources.load_shader("res://shaders/raymarching/raymarcher.vs", "res://shaders/raymarching/raymarcher.fs");

    PerlinNoiseTexture perlinTexture3D(128, 128, 128);
    GLCall(GLuint textureID3D = perlinTexture3D.getTextureID());
    perlin3d = textureID3D;
}

void Raymarcher::tick(float delta) {
    time += delta;
}

void Raymarcher::enqueue(Renderer& renderer, ResourceManager& resources, glm::vec3 camera_pos, glm::vec3 sun_dir, glm::vec3 sun_color, unsigned int voxel_tex, glm::ivec3 grid_dim, glm::vec3 grid_origin, float cell_size) {
    
    if (!_visible) return;

    if (auto shader = resources.get_shader(r_shader.id)) {
        (*shader)->hot_reload_if_changed();
        (*shader)->bind();
        upload_uniforms(renderer, *shader, camera_pos, sun_dir, sun_color, grid_dim, grid_origin, cell_size);

        TextureBinding perlin_noise{ perlin3d, GL_TEXTURE_3D, 5, "u_noise_texture" };
        TextureBinding vox_tex{ voxel_tex, GL_TEXTURE_3D, 6, "u_voxels" };

        RenderCommand cmd{};
        cmd.draw_type = DrawType::FullscreenQuad;
        cmd.shader    = (*shader);
        cmd.state.depth_write = false;
        cmd.textures.push_back(perlin_noise);
        cmd.textures.push_back(vox_tex);
        cmd.attach_lights = true;

        renderer.submit(RenderPass::Volumetrics, cmd);

    } else {
        std::cout << "raymarch shader is fucked" << std::endl;
    }
}

void Raymarcher::upload_uniforms(Renderer& renderer, Shader* shader, glm::vec3 camera_pos, glm::vec3 sun_dir, glm::vec3 sun_color, glm::ivec3 grid_dim, glm::vec3 grid_origin, float cell_size) {
        glm::mat4 proj = renderer.get_proj();
        glm::mat4 view = renderer.get_view();
        glm::mat4 inverted_proj_view = glm::inverse(proj * view);

        // standard uniforms
        shader->set_uniform_mat4("u_invprojview", inverted_proj_view);
        shader->set_uniform_vec3("u_camera_pos", camera_pos);
        shader->set_uniform_vec3("u_sun_dir", sun_dir);
        shader->set_uniform_vec3("u_sun_color", sun_color);
        shader->set_uniform_float("u_time", time);
        shader->set_uniform_vec2("u_resolution", renderer.get_viewport_size());

        // depth textures
        shader->set_uniform_int("u_scene_depth", 2);
        shader->set_uniform_int("u_raymarch_depth", 3);

        // voxel grid params
        shader->set_uniform_ivec3("u_grid_dim", grid_dim);
        shader->set_uniform_vec3("u_grid_origin", grid_origin);
        shader->set_uniform_float("u_cell_size", cell_size);

        // raymarch parameters
        shader->set_uniform_int("u_max_steps", raymarch_settings.max_steps);
        shader->set_uniform_float("u_step_size", raymarch_settings.step_size);
        shader->set_uniform_int("u_max_light_steps", raymarch_settings.max_light_steps);
        shader->set_uniform_float("u_light_step_size", raymarch_settings.light_step_size);
        shader->set_uniform_vec3("u_base_color", raymarch_settings.base_color);
        shader->set_uniform_float("u_absorption_coefficient", raymarch_settings.absorption_coefficient);
        shader->set_uniform_float("u_scattering_coefficient", raymarch_settings.scattering_coefficient);
        shader->set_uniform_float("u_extincion_coefficient", raymarch_settings.extincion_coefficient);
        shader->set_uniform_float("u_anisotropy", raymarch_settings.anisotropy);
        shader->set_uniform_float("u_sun_intensity", raymarch_settings.sun_intensity);
}

void save_perlin() {
    PerlinNoiseTexture perlinTexture2D(512, 512, "/Users/puff/Developer/graphics/Volumetrics/testing/test.ppm");
}
