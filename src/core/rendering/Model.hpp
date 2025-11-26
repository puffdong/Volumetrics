#pragma once

#pragma once

#include <string>
#include <vector>
#include "glm/glm.hpp"

struct ModelGpuData {
    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int ebo = 0;
    int index_count = 0;
    int vertex_count = 0;

    glm::vec3 aabb_min = glm::vec3(0.0f);
    glm::vec3 aabb_max = glm::vec3(0.0f);
    
    std::string name = "";
};
