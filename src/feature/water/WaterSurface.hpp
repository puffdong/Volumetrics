#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>
#include "../../core/rendering/Shader.h"
#include "../../OBJLoader.h"
#include "../../utils/ButtonMap.h"

class WaterSurface {
private:
    Shader* shader;
    ModelObject* model;

    glm::vec3 pos;
    float height;
    float width;

    float time;
    
public:
    WaterSurface(glm::vec3 pos, float height, float width);

    void tick(ButtonMap bm, float delta);

    void render(glm::mat4 proj, glm::mat4 view, glm::vec3 camera_pos);
};