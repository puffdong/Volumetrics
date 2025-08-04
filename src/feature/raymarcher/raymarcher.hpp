#pragma once
#include <GL/glew.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>
#include "../../utils/perlin_noise_generator.hpp"
#include "../../OBJLoader.h"
#include "../../core/rendering/Shader.h" 
#include "rayscene.hpp"

class Raymarcher {
private:
    Shader* shader;
    unsigned int quadVAO, quadVBO; // the vbo for the screenwide polygon
    RayScene* ray_scene;
    GLuint perlin3d;
    float time = 0.0;

    glm::mat4 proj;
    float near_plane, far_plane;

public:
    Raymarcher(RayScene* scene);

    void tick(float delta);

    void render(glm::vec3 camera_pos, glm::mat4 view_matrix, glm::mat4 projMatrix, float near, float far);

    // new
    void update_static_uniforms(glm::mat4 proj, float near, float far);
    void enqueue(RenderPass pass = RenderPass::Forward) const;
};


