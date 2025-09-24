#pragma once
#include "RenderCommand.hpp"
#include "Shader.hpp"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp> // get that perspective thing
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
    // void present_to_screen(); // to be removed
    
    // getters n' setters
    void set_projection_matrix(float aspect_ratio, float fov, float near_plane = 1.0, float far_plane = 256);
    void set_fov(float fov);
    void set_view(glm::mat4 v) { view = v; };
    
    inline glm::mat4 get_proj() const { return proj; };
    inline glm::mat4 get_view() const { return view; }
    inline float get_fov() const { return fov; };
    inline float get_near() const { return near; };
    inline float get_far() const { return far; };

private:
    RenderState current {};

    float fov = 70.f;
	float near = 1.0f; // hmm... what is too close/too far? 
	float far = 256.0f;
	float aspect_ratio = 16.f / 9.0f;
	glm::mat4 proj;
    glm::mat4 view;
	bool changes_made = true;


    // work in progress
    GLuint sceneFBO = 0;
    GLuint sceneColorTex = 0;
    GLuint sceneDepthRBO = 0;

    GLuint volumetrics_fbo = 0;
    GLuint volumetrics_fbo_color = 0;
    GLuint volumetrics_fbo_depth = 0;
    // how do i do up arrow ?! |^|^|^| (boost panel)

    std::vector<RenderCommand> queues[int(RenderPass::Volumetrics)+1]; // bruuuuuh
    void apply_state(RenderState s);
    void execute_command(const RenderCommand& cmd);
    void recreate_scene_fbo(int width, int height);

    void init_quad();
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;

    void init_framebuffer(int width, int height);

    // Shader* test_shader;
};

