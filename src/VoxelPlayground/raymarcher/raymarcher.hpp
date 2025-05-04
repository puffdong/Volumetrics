#pragma once
#include <GL/glew.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>
#include "../../OBJLoader.h"
#include "../../Shader.h" 
#include "rayscene.hpp"

class Raymarcher {
private:
    Shader* shader;
    unsigned int quadVAO, quadVBO;
    RayScene* ray_scene;
    float time = 0.0;

public:
    Raymarcher(RayScene* scene);
    void render(glm::vec3 camera_pos, glm::mat4 view_matrix, float delta);
};


