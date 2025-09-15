#pragma once
#include <GL/glew.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>
#include "core/utils/perlin_noise_generator.hpp"
#include "core/OBJLoader.hpp"
#include "core/rendering/Shader.hpp" 
#include "rayscene.hpp"
#include "core/Camera.hpp"

class Raymarcher {
private:
    Shader* shader;
    RayScene* ray_scene;
    GLuint perlin3d;
    float time = 0.0;

    glm::mat4 proj;
    float near_plane, far_plane;

public:
    Raymarcher(RayScene* scene);

    void tick(float delta);

    // new
    void update_static_uniforms(glm::mat4 proj, float near, float far);
    void enqueue(RenderPass pass, Camera* camera, glm::vec3 sun_dir) const;
};


