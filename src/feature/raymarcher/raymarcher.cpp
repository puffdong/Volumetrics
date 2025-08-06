#include "raymarcher.hpp"
#include <iostream>
#include "../../core/rendering/Renderer.h"
#include <vector>

Raymarcher::Raymarcher(RayScene* scene)
    : quadVAO(0), quadVBO(0), ray_scene(scene)
{
    shader = new Shader("/Users/puff/Developer/graphics/Volumetrics/res/shaders/raymarching/raymarcher.shader");

    float quadVertices[] = { // draw something on the entirety of the screen :)
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f
    };

    GLCall(glGenVertexArrays(1, &quadVAO));
    GLCall(glGenBuffers(1, &quadVBO));
    GLCall(glBindVertexArray(quadVAO));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, quadVBO));
    GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW));
    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));

    std::cout << "Generating perlin noise" << std::endl;

    // GLuint perlin2d = generatePerlin2D();
    PerlinNoiseTexture perlinTexture3D(128, 128, 128);
    GLCall(GLuint textureID3D = perlinTexture3D.getTextureID());

    perlin3d = textureID3D;
    std::cout << "Finished generating perlin noise" << std::endl;

    std::cout << "Raymarcher engaged! VAO: " << quadVAO << " VBO: " << quadVBO << std::endl;
}

void Raymarcher::tick(float delta) {
    time += delta;
}


void Raymarcher::render(glm::vec3 camera_pos, glm::mat4 view_matrix, glm::mat4 projMatrix, float near, float far) {

    glm::mat4 invprojview = glm::inverse(projMatrix * view_matrix);

    shader->HotReloadIfChanged();
    shader->Bind();
    shader->SetUniform1f("time", time);

    shader->SetUniform1f("near_plane", near);
    shader->SetUniform1f("far_plane", far);
    shader->SetUniformMat4("invprojview", invprojview);

    GLCall(glActiveTexture(GL_TEXTURE0));
    GLCall(glBindTexture(GL_TEXTURE_3D, perlin3d));
    shader->SetUniform1i("noise_texture", 0);

    ray_scene->upload_primitives_to_gpu(shader);

    // // GLCall(glDisable(GL_DEPTH_TEST));
    // glDisable(GL_CULL_FACE);
    // glDepthMask(GL_FALSE);
    // glBindVertexArray(quadVAO);
    // glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
    // glDepthMask(GL_TRUE);
    // // GLCall(glEnable(GL_DEPTH_TEST));

    enqueue(RenderPass::Volumetrics);
}

void Raymarcher::update_static_uniforms(glm::mat4 proj, float near, float far) {
    // update only the static uniforms once.
    shader->Bind();
    if ((near_plane != near) && (far_plane != far_plane)) {
        near_plane = near;
        far_plane = far;

        shader->SetUniform1f("near_plane", near_plane);
        shader->SetUniform1f("far_plane", far_plane);
    }
    shader->Unbind();
}

void Raymarcher::enqueue(RenderPass pass) const {
    
    TextureBinding bind{perlin3d, GL_TEXTURE_3D, 0, "noise_texture"};

    RenderCommand cmd{};
    cmd.vao        = quadVAO;
    cmd.draw_type   = DrawType::Arrays;
    cmd.primitive = GL_TRIANGLE_FAN;
    cmd.count      = 4;
    cmd.shader     = shader;
    cmd.state.depth_write = false;
    cmd.textures.push_back(bind);

    Renderer::Submit(pass, cmd);
}
