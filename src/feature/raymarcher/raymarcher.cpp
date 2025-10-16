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
                               glm::vec3(-15, -5, -30), // pos
                               glm::vec3(0.f), // rot
                               glm::vec3(1.f), // scale
                               (Base*) this);
    voxel_grid->init(resources, space);
    _children[voxel_grid->get_id()] = voxel_grid;

    PerlinNoiseTexture perlinTexture3D(128, 128, 128);
    GLCall(GLuint textureID3D = perlinTexture3D.getTextureID());
    perlin3d = textureID3D;

    _ray_scene = new RayScene(glm::vec3(0.0, 0.0, 0.0));
    sphere1 = _ray_scene->add_sphere(glm::vec3(-5.0f, -3.0f, -10.0f), 15.0f, glm::vec4(1.0, 0.98, 0.92, 1.0));
    sphere2 = _ray_scene->add_sphere(glm::vec3(-10.0f, 0.0f, 0.0f), 3.0f, glm::vec4(1.0, 0.98, 0.92, 1.0));
}

void Raymarcher::tick(float delta) {
    time += delta;
    voxel_grid->tick(delta);
    sun_direction = _space->get_sun()->get_direction();
    ui::raymarcher_panel(*this, *voxel_grid);
    

    // sphere1->pos.x = 13 * sin(time * 0.1f);
	// sphere1->pos.z = 13 * cos(time * 0.1f);

	// sphere2->pos.x = 7 * sin(-time * 0.15f);
	// sphere2->pos.z = 7 * cos(-time * 0.15f);
}

void Raymarcher::enqueue(Renderer& renderer, ResourceManager& resources) {
    voxel_grid->enqueue(renderer, resources);
    
    glm::mat4 proj = renderer.get_proj();
    glm::mat4 view = renderer.get_view();
    glm::mat4 inverted_proj_view = glm::inverse(proj * view);

    if (auto shader = resources.get_shader(r_shader.id)) {

        (*shader)->hot_reload_if_changed();
        (*shader)->bind();
        (*shader)->set_uniform_float("u_time", time);
        (*shader)->set_uniform_mat4("u_invprojview", inverted_proj_view);
        (*shader)->set_uniform_float("u_near_plane", renderer.get_near());
        (*shader)->set_uniform_float("u_far_plane", renderer.get_far());
        (*shader)->set_uniform_vec3("u_camera_pos", _space->get_camera()->get_position());
        (*shader)->set_uniform_vec3("u_sun_dir", sun_direction);
        // _ray_scene->upload_primitives_to_gpu((*shader));

        TextureBinding bind{ perlin3d, GL_TEXTURE_3D, 0, "u_noise_texture" };

        (*shader)->set_uniform_ivec3("u_grid_dim", voxel_grid->get_grid_dim());
        (*shader)->set_uniform_vec3("u_grid_origin", voxel_grid->get_position());
        (*shader)->set_uniform_float("u_voxel_size", voxel_grid->get_cell_size());
        TextureBinding bind2{ voxel_grid->get_voxel_texture_id(), GL_TEXTURE_3D, 1, "u_voxels" };

        RenderCommand cmd{};
        cmd.draw_type = DrawType::Framebuffer;
        cmd.shader    = (*shader);
        cmd.state.depth_write = false;
        cmd.textures.push_back(bind);
        cmd.textures.push_back(bind2);

        renderer.submit(RenderPass::Volumetrics, cmd);

    } else {
        std::cout << "raymarch shader is fucked" << std::endl;
    }

}


void save_perlin() {
    PerlinNoiseTexture perlinTexture2D(512, 512, "/Users/puff/Developer/graphics/Volumetrics/testing/test.ppm");
}
