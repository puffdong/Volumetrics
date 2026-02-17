#define TINYOBJLOADER_IMPLEMENTATION
#include "ModelAdapter.hpp"

#include <GL/glew.h>
#include "tiny_obj_loader.h"

#include <iostream>
#include <unordered_map>
#include <tuple>
#include <limits>
#include <filesystem>

namespace ModelAdapter {

    namespace {
        struct TupleHasher {
            std::size_t operator()(const std::tuple<int, int, int>& tuple) const {
                std::size_t h1 = std::hash<int>{}(std::get<0>(tuple));
                std::size_t h2 = std::hash<int>{}(std::get<1>(tuple));
                std::size_t h3 = std::hash<int>{}(std::get<2>(tuple));
                return h1 ^ (h2 << 1) ^ (h3 << 2);
            }
        };
    } // anonymous namespace

    ModelGpuData load_obj(const std::string& file_path) {
        tinyobj::ObjReaderConfig reader_config;
        reader_config.mtl_search_path = "./";

        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile(file_path, reader_config)) {
            if (!reader.Error().empty()) {
                std::cerr << "TinyObjReader Error: " << reader.Error() << std::endl;
            }
            throw std::runtime_error("Failed to load OBJ: " + file_path);
        }

        const auto& attrib = reader.GetAttrib();
        const auto& shapes = reader.GetShapes();
        // const auto& materials = reader.GetMaterials(); // todo: get materials to work!

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        std::unordered_map<std::tuple<int, int, int>, unsigned int, TupleHasher> vertex_map;

        // AABB in model space
        glm::vec3 aabb_min(
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max()
        );
        glm::vec3 aabb_max(
            -std::numeric_limits<float>::max(),
            -std::numeric_limits<float>::max(),
            -std::numeric_limits<float>::max()
        );

        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); ++s) {
            size_t index_offset = 0;

            // Loop over faces (polygons)
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
                const size_t fv = static_cast<size_t>(shapes[s].mesh.num_face_vertices[f]);

                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; ++v) {
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    auto key = std::make_tuple(idx.vertex_index, idx.normal_index, idx.texcoord_index);

                    auto it = vertex_map.find(key);
                    if (it == vertex_map.end()) {
                        // New unique vertex
                        const unsigned int new_index = static_cast<unsigned int>(vertices.size() / 8);
                        vertex_map.emplace(key, new_index);

                        const float vx = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 0];
                        const float vy = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 1];
                        const float vz = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 2];

                        // position
                        vertices.push_back(vx);
                        vertices.push_back(vy);
                        vertices.push_back(vz);

                        // track AABB
                        aabb_min.x = std::min(aabb_min.x, vx);
                        aabb_min.y = std::min(aabb_min.y, vy);
                        aabb_min.z = std::min(aabb_min.z, vz);
                        aabb_max.x = std::max(aabb_max.x, vx);
                        aabb_max.y = std::max(aabb_max.y, vy);
                        aabb_max.z = std::max(aabb_max.z, vz);

                        // normal
                        if (idx.normal_index >= 0) {
                            vertices.push_back(attrib.normals[3 * static_cast<size_t>(idx.normal_index) + 0]);
                            vertices.push_back(attrib.normals[3 * static_cast<size_t>(idx.normal_index) + 1]);
                            vertices.push_back(attrib.normals[3 * static_cast<size_t>(idx.normal_index) + 2]);
                        } else {
                            vertices.push_back(0.0f);
                            vertices.push_back(0.0f);
                            vertices.push_back(0.0f);
                        }

                        // uv
                        if (idx.texcoord_index >= 0) {
                            vertices.push_back(attrib.texcoords[2 * static_cast<size_t>(idx.texcoord_index) + 0]);
                            vertices.push_back(attrib.texcoords[2 * static_cast<size_t>(idx.texcoord_index) + 1]);
                        } else {
                            vertices.push_back(0.0f);
                            vertices.push_back(0.0f);
                        }

                        indices.push_back(new_index);
                    } else {
                        // Reuse existing vertex index
                        indices.push_back(it->second);
                    }
                }

                index_offset += fv;
                // shapes[s].mesh.material_ids[f];
            }
        }

        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint ebo = 0;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                     vertices.data(),
                     GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
                     indices.data(),
                     GL_STATIC_DRAW);

        const GLsizei stride = 8 * static_cast<GLsizei>(sizeof(float));

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(6 * sizeof(float)));

        glBindVertexArray(0);

        ModelGpuData model;
        model.name     = std::filesystem::path(file_path).stem().string();

        model.vao          = vao;
        model.vbo          = vbo;
        model.ebo          = ebo;
        model.index_count  = static_cast<int>(indices.size());
        model.vertex_count = static_cast<int>(vertices.size() / 8);

        model.aabb_min = aabb_min;
        model.aabb_max = aabb_max;

        return model;
    }

}




 