#pragma once
#include "RenderCommand.hpp"
#include "Shader.h"
#include <vector>

// Error handling
// #define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    GLLogCall(#x, __FILE__, __LINE__);
    
    
    // ASSERT(GLLogCall(#x, __FILE__, __LINE__)) 

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);

static RenderState current {};  

// Renderer.hpp
class Renderer
{    
public: 
    static void InitRenderer(int width, int height);
    static void Resize(int width, int height);
    static void BeginFrame(const glm::vec4& clearColor = {0,0,0,1});
    static void Submit(RenderPass pass, const RenderCommand& cmd);
    static void ExecutePipeline();
    static void Flush(RenderPass pass);   // executes queued commands
    static void PresentToScreen();

private:
    static GLuint sceneFBO;
    static GLuint sceneColorTex;
    static GLuint sceneDepthRBO;

    static GLuint volumetrics_fbo;
    static GLuint volumetrics_fbo_color;
    static GLuint volumetrics_fbo_depth;

    static std::vector<RenderCommand> queues[int(RenderPass::Volumetrics)+1]; // bruuuuuh
    static void applyState(RenderState s);
    static void executeCommand(const RenderCommand& cmd);


    static void recreateSceneFBO(int width, int height);

    static void InitQuad();
    static GLuint quadVAO;
    static GLuint quadVBO;

    static void InitFramebuffer(int width, int height);

    static Shader* test_shader;
};

