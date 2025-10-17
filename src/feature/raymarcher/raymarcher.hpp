#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "core/Base.hpp"
// #include "rayscene.hpp"
#include "feature/raymarcher/VoxelGrid.hpp"

class Space;

class Raymarcher : public Base {
private:
    Resource r_shader;

    // RayScene* _ray_scene;
    // RaySphere* sphere1;
	// RaySphere* sphere2;

    VoxelGrid* voxel_grid;

    GLuint perlin3d;

    float time = 0.0;
    glm::vec3 sun_direction = glm::vec3(0.f);

public:
    Raymarcher();
    void init(ResourceManager& resources, Space* space) override;
    void tick(float delta) override;
    void enqueue(Renderer& renderer, ResourceManager& resources) override;
};


