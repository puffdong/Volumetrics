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

public:
    Raymarcher(RayScene* scene);

    void tick(float delta);
    void enqueue(Renderer& renderer, RenderPass pass, Camera* camera, glm::vec3 sun_dir) const;
};


