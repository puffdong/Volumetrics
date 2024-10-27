#pragma once

#include <GL/glew.h>


#include "Shader.h"

#include "OBJLoader.h"

// Error handling
#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__)) 


void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);

class Renderer {
public:
    void Clear() const;
    void DrawObject(Shader& shader, ModelObject& m);
};