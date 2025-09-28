#include "raymarcher.hpp"
#include <iostream>
#include "core/rendering/Renderer.hpp"
#include <vector>

Raymarcher::Raymarcher(RayScene* scene)
    : ray_scene(scene)
{
    shader = new Shader("/Users/puff/Developer/graphics/Volumetrics/res/shaders/raymarching/volumetric_marcher.shader");
    PerlinNoiseTexture perlinTexture3D(128, 128, 128);
    GLCall(GLuint textureID3D = perlinTexture3D.getTextureID());

    perlin3d = textureID3D;
}

void Raymarcher::tick(float delta) {
    time += delta;
}


void Raymarcher::enqueue(Renderer& renderer, RenderPass pass, Camera* camera, glm::vec3 sun_dir) const {
    glm::mat4 invprojview = glm::inverse(renderer.get_proj() * camera->get_view_matrix());

    shader->HotReloadIfChanged();
    shader->Bind();
    shader->SetUniform1f("time", time);
    shader->SetUniformMat4("invprojview", invprojview);
    shader->SetUniform1f("near_plane", renderer.get_near());
    shader->SetUniform1f("far_plane", renderer.get_far());
    shader->SetUniform3f("camera_pos", camera->get_position());
    shader->SetUniform3f("sun_dir", sun_dir); 

    ray_scene->upload_primitives_to_gpu(shader);

    TextureBinding bind{ perlin3d, GL_TEXTURE_3D, 0, "noise_texture" };

    RenderCommand cmd{};
    cmd.draw_type = DrawType::Framebuffer;
    cmd.shader    = shader;
    cmd.state.depth_write = false;
    cmd.textures.push_back(bind);

    renderer.submit(pass, cmd);
}


void save_perlin() {
    PerlinNoiseTexture perlinTexture2D(512, 512, "/Users/puff/Developer/graphics/Volumetrics/testing/test.ppm");
}
