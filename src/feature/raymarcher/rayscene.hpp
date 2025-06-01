#pragma once
#include "glm/glm.hpp"
#include "../../rendering/Shader.h"
#include <vector>

struct RaySphere {
    glm::vec3 pos;
    float radius;
    glm::vec4 color;
};

struct RayTorus {
    glm::vec3 pos;
    glm::vec2 t;
    glm::vec3 color;
};

struct VolumetricCube {
    int texture_id; // gl_int to the generated 3D texture
    glm::vec3 origin;
    glm::vec3 dimensions;
};

class RayScene {
private:
    glm::vec3 origin;
    std::vector<RaySphere*> spheres;
    std::vector<RayTorus*> toruses;

    VolumetricCube* vol_cube = nullptr;

public:
    RayScene(glm::vec3 world_origin);
    VolumetricCube* add_volumetric_cube(glm::vec3 position, glm::vec3 dimemsions);
    RaySphere* add_sphere(glm::vec3 position, float radius, glm::vec4 color);
    // void add_cube();
    void add_torus(glm::vec3 position, glm::vec2 t, glm::vec4 color);
    void upload_primitives_to_gpu(Shader* shader);
    void upload_volumetric_box_to_gpu(Shader* shader);

};
