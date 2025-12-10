#include "raymarcher.hpp"
#include <iostream>
#include "core/space/Space.hpp"
#include "core/rendering/Renderer.hpp"
#include "core/utils/perlin_noise_generator.hpp"
#include "core/ui/ui_dumptruck.hpp"



Raymarcher::Raymarcher() : Base()
{

}

void Raymarcher::init(ResourceManager& resources, Space* space) {
    Base::init(resources, space);
    std::cout << "raymarcher init" << std::endl;
    r_shader = resources.load_shader("res://shaders/raymarching/raymarcher.vs", "res://shaders/raymarching/raymarcher.fs");

    voxel_grid = new VoxelGrid(30, 30, 30, 0, 1.5f, 
                               glm::vec3(0.0, 0.0, 0.0), // pos
                               glm::vec3(0.f), // rot
                               glm::vec3(1.f), // scale
                               (Base*) this);
    voxel_grid->init(resources, space);
    voxel_grid->set_visibility(false);
    _children[voxel_grid->get_id()] = voxel_grid;

    PerlinNoiseTexture perlinTexture3D(128, 128, 128);
    GLCall(GLuint textureID3D = perlinTexture3D.getTextureID());
    perlin3d = textureID3D;
}

void Raymarcher::tick(float delta) {
    time += delta;
    ui::raymarcher_panel(*this, raymarch_settings, *voxel_grid);
    voxel_grid->tick(delta);
}

void Raymarcher::enqueue(Renderer& renderer, ResourceManager& resources) {
    voxel_grid->enqueue(renderer, resources);
    
    if (!_visible) return;

    if (auto shader = resources.get_shader(r_shader.id)) {
        (*shader)->hot_reload_if_changed();
        (*shader)->bind();
        upload_uniforms(renderer, *shader);

        TextureBinding perlin_noise{ perlin3d, GL_TEXTURE_3D, 5, "u_noise_texture" };

        TextureBinding voxel_tex{ voxel_grid->get_voxel_texture_id(), GL_TEXTURE_3D, 6, "u_voxels" };

        RenderCommand cmd{};
        cmd.draw_type = DrawType::FullscreenQuad;
        cmd.shader    = (*shader);
        cmd.state.depth_write = false;
        cmd.textures.push_back(perlin_noise);
        cmd.textures.push_back(voxel_tex);
        cmd.attach_lights = true;

        renderer.submit(RenderPass::Volumetrics, cmd);

    } else {
        std::cout << "raymarch shader is fucked" << std::endl;
    }
}

void Raymarcher::upload_uniforms(Renderer& renderer, Shader* shader) {
        glm::mat4 proj = renderer.get_proj();
        glm::mat4 view = renderer.get_view();
        glm::mat4 inverted_proj_view = glm::inverse(proj * view);

        // standard uniforms
        shader->set_uniform_mat4("u_invprojview", inverted_proj_view);
        shader->set_uniform_float("u_near_plane", renderer.get_near());
        shader->set_uniform_float("u_far_plane", renderer.get_far());
        shader->set_uniform_vec3("u_camera_pos", _space->get_camera()->get_position());
        shader->set_uniform_vec3("u_sun_dir", _space->get_sun()->get_direction());
        shader->set_uniform_vec3("u_sun_color", _space->get_sun()->get_color());
        shader->set_uniform_float("u_time", time);

        // voxel grid params
        shader->set_uniform_ivec3("u_grid_dim", voxel_grid->get_grid_dim());
        shader->set_uniform_vec3("u_grid_origin", voxel_grid->get_position());
        shader->set_uniform_float("u_cell_size", voxel_grid->get_cell_size());

        // raymarch parame
        shader->set_uniform_int("u_max_steps", raymarch_settings.max_steps);
        shader->set_uniform_float("u_step_size", raymarch_settings.step_size);
        shader->set_uniform_int("u_max_light_steps", raymarch_settings.max_light_steps);
        shader->set_uniform_float("u_hit_step_size", raymarch_settings.hit_step_size);
        shader->set_uniform_float("u_light_step_size", raymarch_settings.light_step_size);
        shader->set_uniform_float("u_max_distance", raymarch_settings.max_distance);
        shader->set_uniform_float("u_min_distance", raymarch_settings.min_distance);
        shader->set_uniform_vec3("u_base_color", raymarch_settings.base_color);
        shader->set_uniform_float("u_absorption_coefficient", raymarch_settings.absorption_coefficient);
        shader->set_uniform_float("u_scattering_coefficient", raymarch_settings.scattering_coefficient);
        shader->set_uniform_float("u_extincion_coefficient", raymarch_settings.extincion_coefficient);
}

void save_perlin() {
    PerlinNoiseTexture perlinTexture2D(512, 512, "/Users/puff/Developer/graphics/Volumetrics/testing/test.ppm");
}
