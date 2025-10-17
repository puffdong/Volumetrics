#pragma once
#include "core/Base.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <iostream>

struct LinePrimitive {
    glm::vec3 start;
    glm::vec3 end;
};

class Line : public Base {
private:
    std::vector<LinePrimitive> line_primitives;
    int num_lines;

    Resource r_shader;

    // Shader* shader;
    unsigned int VAO, VBO, instanceVBO;

public:
    Line(const glm::vec3& start,
         const glm::vec3& end,
         const glm::vec3& pos = glm::vec3(0.0f),
         const glm::vec3& rot = glm::vec3(0.0f),
         const glm::vec3& sc  = glm::vec3(1.0f));

    Line(std::vector<LinePrimitive> lines,
         const glm::vec3& pos = glm::vec3(0.0f),
         const glm::vec3& rot = glm::vec3(0.0f),
         const glm::vec3& sc  = glm::vec3(1.0f));

    ~Line() override;

    void init(ResourceManager& resources, Space* space) override;
    void tick(float delta) override;
    void enqueue(Renderer& renderer, ResourceManager& resources) override;
};