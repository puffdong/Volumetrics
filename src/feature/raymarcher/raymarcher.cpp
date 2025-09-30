#include "raymarcher.hpp"
#include <iostream>
#include "core/space/Space.hpp"
#include "core/rendering/Renderer.hpp"
#include "core/utils/perlin_noise_generator.hpp"



Raymarcher::Raymarcher() : Base()
{

}

void Raymarcher::init(ResourceManager& resources, Space* space) {
    Base::init(resources, space);
    r_shader = resources.load_shader("res://shaders/raymarching/volumetric_marcher.shader");

    voxel_grid = new VoxelGrid(20, 20, 20, 1, 0.75f, 
                               glm::vec3(20, 20, 20), // pos
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
        (*shader)->SetUniform1f("time", time);
        (*shader)->SetUniformMat4("invprojview", inverted_proj_view);
        (*shader)->SetUniform1f("near_plane", renderer.get_near());
        (*shader)->SetUniform1f("far_plane", renderer.get_far());
        (*shader)->SetUniform3f("camera_pos", _space->get_camera()->get_position());
        (*shader)->SetUniform3f("sun_dir", sun_direction);
        _ray_scene->upload_primitives_to_gpu((*shader));

        TextureBinding bind{ perlin3d, GL_TEXTURE_3D, 0, "noise_texture" };

        RenderCommand cmd{};
        cmd.draw_type = DrawType::Framebuffer;
        cmd.shader    = (*shader);
        cmd.state.depth_write = false;
        cmd.textures.push_back(bind);

        renderer.submit(RenderPass::Volumetrics, cmd);

    } else {
        std::cout << "raymarch shader is fucked" << std::endl;
    }

}


void save_perlin() {
    PerlinNoiseTexture perlinTexture2D(512, 512, "/Users/puff/Developer/graphics/Volumetrics/testing/test.ppm");
}
