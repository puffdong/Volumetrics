#include "rayscene.hpp"
#include "../../utils/perlin_noise_generator.hpp"
#include <iostream>

RayScene::RayScene(glm::vec3 world_origin) : origin(world_origin) {

}

VolumetricCube* RayScene::add_volumetric_cube(glm::vec3 position, glm::vec3 dimemsions) {
    if (vol_cube = nullptr) {
        std::cout << "Generating perlin noise" << std::endl;
        auto perlin = PerlinNoiseTexture(128, 128, 128);
        int textureID = perlin.getTextureID();
        std::cout << "Finished generating perlin noise. Texture ID: " << textureID << std::endl;

        vol_cube = new VolumetricCube{ textureID, position, dimemsions };
    }
    return vol_cube;

}

RaySphere* RayScene::add_sphere(glm::vec3 position, float radius, glm::vec4 color) {
    // auto new_raysphere = RaySphere{position, radius, color};

    // spheres.push_back(std::move(new_raysphere));

    auto new_raysphere = new RaySphere{ position, radius, color };

    spheres.push_back(new_raysphere);
    return new_raysphere;
}

void RayScene::add_torus(glm::vec3 position, glm::vec2 t, glm::vec4 color) {
    // auto new_torus = RayTorus{position, t, color};

    // toruses.push_back(std::move(new_torus));

    auto new_torus = new RayTorus{ position, t, color };

    toruses.push_back(new_torus);
}

void RayScene::upload_primitives_to_gpu(Shader* shader) {
    std::vector<glm::vec3> sphere_positions;
    std::vector<float> sphere_radiuses;
    std::vector<glm::vec4> sphere_colors;

    for (RaySphere* sphere : spheres) {
        sphere_positions.push_back(sphere->pos);
        sphere_radiuses.push_back(sphere->radius);
        sphere_colors.push_back(sphere->color);
    };

    shader->SetUniform3fv("sphere_positions", sphere_positions);
    shader->SetUniform4fv("sphere_colors", sphere_colors);
    shader->SetUniform1fv("sphere_radiuses", sphere_radiuses);
    shader->SetUniform1i("num_spheres", (int)spheres.size());
}

void RayScene::upload_volumetric_box_to_gpu(Shader* shader) {

}