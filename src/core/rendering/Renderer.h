#pragma once
#include "RenderCommand.hpp"
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
    static void BeginFrame(const glm::vec4& clearColor = {0,0,0,1});
    static void Submit(RenderPass pass, const RenderCommand& cmd);
    static void Flush(RenderPass pass);   // executes queued commands
    static void EndFrame();               // will handle swap‑buffers outsid
private:
    static std::vector<RenderCommand> queues[int(RenderPass::UI)+1];
    static void applyState(RenderState s);
    static void executeCommand(const RenderCommand& cmd);
};
