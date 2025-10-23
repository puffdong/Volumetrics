#pragma once
#include "Shader.hpp"
#include "glm/glm.hpp"

enum class DrawType { Arrays, Elements, ArraysInstanced, ElementsInstanced, FullscreenQuad };

enum class RenderPass { Skypass, Forward, Volumetrics, UI };

struct RenderState {
    bool depth_test   = true;   // GL_DEPTH_TEST
    bool depth_write  = true;   // glDepthMask
    bool cull_face    = true;   // GL_CULL_FACE
    bool line_smooth  = true;   // GL_LINE_SMOOTH
    bool scissor_test = true;   // GL_SCISSOR_TEST
    
    GLenum cull_front_back = GL_BACK; // glCullFace
};

struct TextureBinding
{
    GLuint  id        = 0;
    GLenum  target    = GL_TEXTURE_2D;    // 2D, 3D, CUBE_MAP…
    uint8_t unit      = 0;                // 0..31
    const char* uniform_name = nullptr;    // "u_Texture" (can be null if always the same)
};

struct RenderCommand
{
    GLuint vao               = 0;
    DrawType draw_type        = DrawType::Arrays;
    GLenum primitive         = GL_TRIANGLES;     // lines, points, etc.
    GLsizei count            = 0;                // vertices or indices
    GLsizei instance_count    = 1;               // >1 ⇒ instanced
    Shader* shader           = nullptr;          
    glm::mat4 model          = glm::mat4(1.0f);  
    std::vector<TextureBinding> textures; // feel like this is wonky
    RenderState state;
};