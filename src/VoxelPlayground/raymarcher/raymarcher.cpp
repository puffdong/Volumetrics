#include "raymarcher.hpp"
#include <iostream>
#include "../../Renderer.h"
#include <vector>

Raymarcher::Raymarcher()
    : quadVAO(0), quadVBO(0)
{
    shader = new Shader("C:/Dev/OpenGL/Volumetrics/res/shaders/raymarching/raymarcher.shader");

    float quadVertices[] = { 
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f
    };
    
    GLCall(glGenVertexArrays(1, &quadVAO));
    GLCall(glGenBuffers(1, &quadVBO));
    GLCall(glBindVertexArray(quadVAO));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, quadVBO));
    GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW));
    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));

    std::cout << "Raymarcher engaged! VAO: " << quadVAO << " VBO: "<< quadVBO << std::endl;
}


void Raymarcher::render(glm::vec3 camera_pos, glm::mat4 view_matrix) {
    shader->Bind();
    shader->SetUniform2f("iResolution", glm::vec2(1600, 900));
    shader->SetUniform3f("camera_pos", camera_pos);

    shader->SetUniformMat4("view_matrix", view_matrix);

    GLCall(glDisable(GL_DEPTH_TEST));
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    GLCall(glEnable(GL_DEPTH_TEST));
}