#pragma once
#include "core/rendering/Model.hpp"
#include <GL/glew.h>

namespace ModelGenerator {
    ModelGpuData create_flat_ground(float width, float depth, int num_rows, int num_cols) {
        float quad_width = width / static_cast<float>(num_cols);
        float quad_depth = depth / static_cast<float>(num_rows);

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        for (int row = 0; row < num_rows; ++row) {
            for (int col = 0; col < num_cols; ++col) {
                float x = col * quad_width - width / 2.0f;
                float z = row * quad_depth - depth / 2.0f;

                vertices.insert(vertices.end(), {           // normals 
                    x, 0.0f, z,                             0.0f, 1.0f, 0.0f,   static_cast<float>(col) / num_cols, static_cast<float>(row) / num_rows,
                    x + quad_width, 0.0f, z,                0.0f, 1.0f, 0.0f,   static_cast<float>(col + 1) / num_cols, static_cast<float>(row) / num_rows,
                    x + quad_width, 0.0f, z + quad_depth,   0.0f, 1.0f, 0.0f,   static_cast<float>(col + 1) / num_cols, static_cast<float>(row + 1) / num_rows,
                    x, 0.0f, z + quad_depth,                0.0f, 1.0f, 0.0f,   static_cast<float>(col) / num_cols, static_cast<float>(row + 1) / num_rows});

                // indices
                unsigned int base_index = (row * num_cols + col) * 4;
                indices.insert(indices.end(), {
                    base_index + 0, base_index + 3, base_index + 2, // Triangle 1 (v0, v3, v2) - CCW
                    base_index + 0, base_index + 2, base_index + 1  // Triangle 2 (v0, v2, v1) - CCW
                    });
            }
        }

        ModelGpuData gpu_data;
        gpu_data.name = "Generated Flat Ground";

        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint ebo = 0;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        GLsizei stride = 8 * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(6 * sizeof(float)));

        glBindVertexArray(0);

        Primitive primitive;
        primitive.vao = vao;
        primitive.index_count = static_cast<int>(indices.size());
        primitive.vertex_count = static_cast<int>(vertices.size() / 8);
        primitive.index_type = GL_UNSIGNED_INT;
        primitive.index_byte_offset = 0;

        Mesh mesh;
        mesh.name = "Ground";
        mesh.primitives.push_back(primitive);

        MeshInstance instance;
        instance.mesh_index = 0;
        instance.transform = glm::mat4(1.0f);

        gpu_data.meshes.push_back(std::move(mesh));
        gpu_data.instances.push_back(instance);
        gpu_data.shared_buffers.push_back(vbo);
        gpu_data.shared_buffers.push_back(ebo);

        return gpu_data;
    }

}

