#pragma once
#include "RenderCommand.hpp"
#include "Shader.hpp"
#include <vector>

// Error handling
// #define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    GLLogCall(#x, __FILE__, __LINE__);
    // ASSERT(GLLogCall(#x, __FILE__, __LINE__)) 

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);

class Renderer
{    
public: 
    void init_renderer(int width, int height);
    void resize(int width, int height);
    void begin_frame(const glm::vec4& clearColor = {0,0,0,1});
    void submit(RenderPass pass, const RenderCommand& cmd);
    void execute_pipeline(); // kinda about to be changed
    void flush(RenderPass pass);
    void present_to_screen(); // to be removed

private:
    RenderState current {};
    GLuint sceneFBO = 0;
    GLuint sceneColorTex = 0;
    GLuint sceneDepthRBO = 0;

    GLuint volumetrics_fbo = 0;
    GLuint volumetrics_fbo_color = 0;
    GLuint volumetrics_fbo_depth = 0;

    std::vector<RenderCommand> queues[int(RenderPass::Volumetrics)+1]; // bruuuuuh
    void apply_state(RenderState s);
    void execute_command(const RenderCommand& cmd);


    void recreate_scene_fbo(int width, int height);

    void init_quad();
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;

    void init_framebuffer(int width, int height);

    Shader* test_shader;
};

