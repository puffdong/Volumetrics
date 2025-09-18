#pragma once
#include "../core/rendering/Shader.hpp"
#include <vector>
#include <glm/glm.hpp>
#include "../core/rendering/Renderer.hpp"

struct LinePrimitive {
    glm::vec3 start;
    glm::vec3 end;
};

class Line {
private:
    std::vector<LinePrimitive> line_primitives;
    int num_lines;

    Shader* shader;
    unsigned int VAO, VBO, instanceVBO;

public:
    Line(glm::vec3 start, glm::vec3 end);
    Line(std::vector<LinePrimitive> lines);
    ~Line();
    void render(Renderer& renderer, glm::mat4 view);
    void enqueue(Renderer& renderer, RenderPass pass = RenderPass::Forward) const;

private:
    void init_render_stuff();
};