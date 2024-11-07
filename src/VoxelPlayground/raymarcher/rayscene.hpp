#include "glm/glm.hpp"
#include "../../Shader.h"
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

class RayScene {
private:
    glm::vec3 origin;
    std::vector<RaySphere> spheres;
    std::vector<RayTorus> toruses;

public:
    RayScene(glm::vec3 world_origin);
    void add_sphere(glm::vec3 position, float radius, glm::vec4 color);
    // void add_cube();
    void add_torus(glm::vec3 position, glm::vec2 t, glm::vec4 color);
    void upload_scene_to_gpu(Shader* shader);

     

};
