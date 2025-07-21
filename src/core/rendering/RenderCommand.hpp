#pragma once
#include "Shader.h"
#include "glm/glm.hpp"

enum class DrawType { Arrays, Elements, ArraysInstanced, ElementsInstanced };

enum class RenderPass { Forward, PostProcess, UI };

struct RenderCommand
{
    GLuint vao               = 0;
    DrawType drawType        = DrawType::Arrays;
    GLenum primitive         = GL_TRIANGLES;     // lines, points, etc.
    GLsizei count            = 0;                // vertices or indices
    GLsizei instanceCount    = 1;                // >1 ⇒ instanced
    Shader* shader           = nullptr;          // raw ptr is fine for now
    glm::mat4 model          = glm::mat4(1.0f);  // per‑object matrix
    // Add more “material” data later (textures, colors…)
};
