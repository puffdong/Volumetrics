#include "Line.hpp"

Line::Line(glm::vec3 start, glm::vec3 end) {
    line_primitives.reserve(2);
    line_primitives.push_back({start, end});
    num_lines = 1;
    init_render_stuff();
}

Line::Line(std::vector<LinePrimitive> lines) {
    for (int i = 0; i < lines.size(); i++) {
        line_primitives.push_back(lines[i]);
    }
    num_lines = lines.size();
    init_render_stuff();
}

Line::~Line() {
    delete shader;
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &instanceVBO);
}

void Line::init_render_stuff() {
    shader = new Shader("/Users/puff/Developer/graphics/Volumetrics/res/shaders/Line.shader");
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(VAO);

    // base line used for interpolating
    float vertices[] = {
        0.0f,
        1.0f
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(LinePrimitive) * num_lines, &line_primitives[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LinePrimitive), (void*)offsetof(LinePrimitive, start));
    glVertexAttribDivisor(1, 1); // gotta let opengl know this is an instanced vertex attribute.

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(LinePrimitive), (void*)offsetof(LinePrimitive, end));
    glVertexAttribDivisor(2, 1); 


    // Unbind the VAO
    glBindVertexArray(0);
}

void Line::render(glm::mat4 proj, glm::mat4 view) {
    shader->HotReloadIfChanged();
    shader->Bind();
    glEnable(GL_LINE_SMOOTH);
    shader->SetUniformMat4("projection", proj);
    shader->SetUniformMat4("view", view);
    glBindVertexArray(VAO);
    glDrawArraysInstanced(GL_LINES, 0, 2, num_lines);
    glBindVertexArray(0);
    glDisable(GL_LINE_SMOOTH);
}