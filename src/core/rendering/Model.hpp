#pragma once
#include <string>
#include <vector>
#include "glm/glm.hpp"
#include <GL/glew.h>

struct Material {
    glm::vec4 base_color_factor = glm::vec4(1.0f);
    GLuint base_color_texture = 0;
    
    // todo
    // GLuint normal_texture = 0;
    // GLuint metallic_roughness_texture = 0;
};

struct MeshInstance {
    int mesh_index;
    glm::mat4 transform;
};

struct Primitive {
    GLuint vao = 0;
    int index_count = 0;
    GLenum index_type = GL_UNSIGNED_INT;
    size_t index_byte_offset = 0;
    int material_index = -1;
};

struct Mesh {
    std::string name;
    std::vector<Primitive> primitives;
};

struct ModelGpuData2 {
    std::string name;
    std::vector<Mesh> meshes;
    std::vector<MeshInstance> instances;
    std::vector<Material> materials;
    std::vector<GLuint> loaded_textures;
    std::vector<GLuint> shared_buffers;
    
    glm::vec3 aabb_min = glm::vec3(0.0f);
    glm::vec3 aabb_max = glm::vec3(0.0f);
};


struct ModelGpuData {
    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int ebo = 0;
    int index_count = 0;
    int vertex_count = 0;

    glm::vec3 aabb_min = glm::vec3(0.0f);
    glm::vec3 aabb_max = glm::vec3(0.0f);
    
    std::string name = "";
};
