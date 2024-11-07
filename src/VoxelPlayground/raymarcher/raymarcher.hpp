#pragma once
#include <GL/glew.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>
#include "../../OBJLoader.h"
#include "../../Shader.h" 

struct RaySphere {
    glm::vec3 pos;
    float radius;
    glm::vec3 color;
};

class Raymarcher {
private:
    Shader* shader;
    unsigned int quadVAO, quadVBO;

public:
    Raymarcher();
    void render(glm::vec3 camera_pos, glm::mat4 view_matrix);
};


