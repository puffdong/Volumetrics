#include "rayscene.hpp"


RayScene::RayScene(glm::vec3 world_origin) : origin(world_origin) {
    add_sphere(glm::vec3(10.0f, 5.0f, 0.0f), 5.0f, glm::vec4(1.0, 0.4, 0.1, 0.8));
    add_sphere(glm::vec3(-10.0f, 6.0f, 1.0f), 3.0f, glm::vec4(1.0, 0.4, 0.1, 0.8));
}

void RayScene::add_sphere(glm::vec3 position, float radius, glm::vec4 color) {
    auto new_raysphere = RaySphere{position, radius, color};

    spheres.push_back(std::move(new_raysphere));
}

void RayScene::add_torus(glm::vec3 position, glm::vec2 t, glm::vec4 color) {
    auto new_torus = RayTorus{position, t, color};

    toruses.push_back(std::move(new_torus));
}

void RayScene::upload_scene_to_gpu(Shader* shader) {
    std::vector<glm::vec3> sphere_positions;
    std::vector<float> sphere_radiuses;
    std::vector<glm::vec4> sphere_colors;

    for (RaySphere sphere : spheres) {
        sphere_positions.push_back(sphere.pos);
        sphere_radiuses.push_back(sphere.radius);
        sphere_colors.push_back(sphere.color);

    }

    shader->SetUniform3fv("sphere_positions", sphere_positions);
    shader->SetUniform4fv("sphere_colors", sphere_colors);
    shader->SetUniform1fv("sphere_radiuses", sphere_radiuses);
}