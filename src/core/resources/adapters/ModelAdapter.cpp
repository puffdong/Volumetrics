#define TINYOBJLOADER_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ModelAdapter.hpp"

#include <GL/glew.h>
#include "tiny_obj_loader.h"
#include "tiny_gltf.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>
#include <unordered_map>
#include <map>
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

    // Helper to calculate the node's matrix and apply the parent's matrix
    void process_node(const tinygltf::Model& model, int node_index, const glm::mat4& parent_matrix, ModelGpuData2& result) {
        const tinygltf::Node& node = model.nodes[node_index];
        glm::mat4 local_matrix(1.0f);

        // glTF specifies a node has EITHER a matrix OR Translation/Rotation/Scale
        if (node.matrix.size() == 16) {
            // It's a raw 4x4 matrix
            local_matrix = glm::make_mat4(node.matrix.data());
        } else {
            // It's TRS (Translation, Rotation, Scale)
            glm::vec3 translation(0.0f);
            glm::quat rotation(1.0f, 0.0f, 0.0f, 0.0f); // w, x, y, z
            glm::vec3 scale(1.0f);

            if (node.translation.size() == 3) {
                translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
            }
            if (node.rotation.size() == 4) {
                // glTF stores quaternions as X, Y, Z, W
                rotation = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
            }
            if (node.scale.size() == 3) {
                scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
            }

            // Build the local matrix: T * R * S
            glm::mat4 mat_t = glm::translate(glm::mat4(1.0f), translation);
            glm::mat4 mat_r = glm::toMat4(rotation);
            glm::mat4 mat_s = glm::scale(glm::mat4(1.0f), scale);
            local_matrix = mat_t * mat_r * mat_s;
        }

        // Multiply by parent to get the final world-space transform
        glm::mat4 global_matrix = parent_matrix * local_matrix;

        // If this node actually contains a mesh, add it to our render list!
        if (node.mesh >= 0) {
            MeshInstance instance;
            instance.mesh_index = node.mesh;
            instance.transform = global_matrix;
            result.instances.push_back(instance);
        }

        // Recursively process all children
        for (int child_index : node.children) {
            process_node(model, child_index, global_matrix, result);
        }
    }

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

    // Helper to convert tinygltf types to OpenGL size (e.g., VEC3 -> 3)
    int GetNumComponents(int type) {
        if (type == TINYGLTF_TYPE_SCALAR) return 1;
        if (type == TINYGLTF_TYPE_VEC2) return 2;
        if (type == TINYGLTF_TYPE_VEC3) return 3;
        if (type == TINYGLTF_TYPE_VEC4) return 4;
        return 1;
    }

    ModelGpuData2 load_gltf(const std::string& file_path) {
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string err, warn;

        bool is_binary = file_path.size() >= 4 && file_path.substr(file_path.size() - 4) == ".glb";
        bool ret = is_binary ? loader.LoadBinaryFromFile(&model, &err, &warn, file_path) 
                             : loader.LoadASCIIFromFile(&model, &err, &warn, file_path);

        if (!warn.empty()) std::cout << "glTF Warn: " << warn << "\n";
        if (!err.empty()) std::cerr << "glTF Err: " << err << "\n";
        if (!ret) throw std::runtime_error("Failed to load glTF: " + file_path);

        ModelGpuData2 result;
        result.name = std::filesystem::path(file_path).stem().string();
        result.aabb_min = glm::vec3(std::numeric_limits<float>::max());
        result.aabb_max = glm::vec3(-std::numeric_limits<float>::max());

        // --- STEP 1: Upload all BufferViews to OpenGL instantly ---
        // glTF organizes data into BufferViews. A BufferView maps perfectly to an OpenGL VBO/EBO.
        result.shared_buffers.resize(model.bufferViews.size());
        glGenBuffers(static_cast<GLsizei>(model.bufferViews.size()), result.shared_buffers.data());

        for (size_t i = 0; i < model.bufferViews.size(); ++i) {
            const tinygltf::BufferView& bufferView = model.bufferViews[i];
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

            // We bind it as GL_ARRAY_BUFFER for now. OpenGL lets us use it as an EBO later if needed.
            glBindBuffer(GL_ARRAY_BUFFER, result.shared_buffers[i]);
            glBufferData(GL_ARRAY_BUFFER, 
                         bufferView.byteLength, 
                         &buffer.data[bufferView.byteOffset], 
                         GL_STATIC_DRAW);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // --- NEW STEP: Upload Textures to OpenGL ---
        std::vector<GLuint> gl_textures(model.textures.size(), 0);
        
        for (size_t i = 0; i < model.textures.size(); ++i) {
            const tinygltf::Texture& gltf_tex = model.textures[i];
            
            // Textures point to an Image. The Image contains the actual pixels.
            if (gltf_tex.source < 0) continue; 
            const tinygltf::Image& image = model.images[gltf_tex.source];

            GLuint tex_id;
            glGenTextures(1, &tex_id);
            glBindTexture(GL_TEXTURE_2D, tex_id);

            // Basic texture parameters (Repeat, Linear filtering)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // Determine if it's RGB or RGBA based on the component count (3 or 4)
            GLenum format = (image.component == 4) ? GL_RGBA : GL_RGB;

            // Upload the pixel data provided by tinygltf
            glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0, format, GL_UNSIGNED_BYTE, &image.image[0]);
            glGenerateMipmap(GL_TEXTURE_2D);

            gl_textures[i] = tex_id;
            result.loaded_textures.push_back(tex_id); // Save it so we can clean it up on destruction
        }

        // --- NEW STEP: Parse Materials ---
        for (const auto& gltf_mat : model.materials) {
            Material engine_mat;

            // Grab the Base Color Factor (a tint applied over the texture, or just a solid color)
            if (gltf_mat.pbrMetallicRoughness.baseColorFactor.size() == 4) {
                engine_mat.base_color_factor = glm::vec4(
                    gltf_mat.pbrMetallicRoughness.baseColorFactor[0],
                    gltf_mat.pbrMetallicRoughness.baseColorFactor[1],
                    gltf_mat.pbrMetallicRoughness.baseColorFactor[2],
                    gltf_mat.pbrMetallicRoughness.baseColorFactor[3]
                );
            }

            // Grab the Base Color Texture index
            int tex_index = gltf_mat.pbrMetallicRoughness.baseColorTexture.index;
            if (tex_index >= 0 && tex_index < gl_textures.size()) {
                engine_mat.base_color_texture = gl_textures[tex_index];
            }

            result.materials.push_back(engine_mat);
        }

        // --- STEP 2: Configure VAOs for each Primitive ---
        // Map glTF attribute strings to your shader layout locations
        std::map<std::string, GLuint> attrib_locations = {
            {"POSITION", 0},
            {"NORMAL", 1},
            {"TEXCOORD_0", 2}
        };

        for (const auto& gltf_mesh : model.meshes) {
            Mesh engine_mesh;
            engine_mesh.name = gltf_mesh.name;

            for (const auto& gltf_primitive : gltf_mesh.primitives) {
                Primitive engine_primitive;
                engine_primitive.material_index = gltf_primitive.material;

                glGenVertexArrays(1, &engine_primitive.vao);
                glBindVertexArray(engine_primitive.vao);

                // 1. Setup Element Buffer Object (Indices)
                if (gltf_primitive.indices >= 0) {
                    const tinygltf::Accessor& accessor = model.accessors[gltf_primitive.indices];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];

                    // Bind the previously uploaded buffer as our EBO
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, result.shared_buffers[accessor.bufferView]);
                    
                    engine_primitive.index_count = static_cast<int>(accessor.count);
                    engine_primitive.index_type = accessor.componentType; // E.g., GL_UNSIGNED_SHORT
                    // accessor.byteOffset is the starting point inside the BufferView!
                    engine_primitive.index_byte_offset = accessor.byteOffset; 
                }

                // 2. Setup Vertex Attributes (Position, Normal, UV)
                for (const auto& attrib : gltf_primitive.attributes) {
                    auto it = attrib_locations.find(attrib.first);
                    if (it == attrib_locations.end()) continue; // Ignore attributes we don't support yet (like TANGENT)

                    GLuint location = it->second;
                    const tinygltf::Accessor& accessor = model.accessors[attrib.second];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];

                    glBindBuffer(GL_ARRAY_BUFFER, result.shared_buffers[accessor.bufferView]);

                    int size = GetNumComponents(accessor.type);
                    
                    // If byteStride is 0, the data is tightly packed. We calculate stride manually.
                    GLsizei stride = bufferView.byteStride == 0 ? 
                                     (size * tinygltf::GetComponentSizeInBytes(accessor.componentType)) : 
                                     static_cast<GLsizei>(bufferView.byteStride);

                    glEnableVertexAttribArray(location);
                    glVertexAttribPointer(location, 
                                          size, 
                                          accessor.componentType, 
                                          accessor.normalized ? GL_TRUE : GL_FALSE, 
                                          stride, 
                                          reinterpret_cast<void*>(accessor.byteOffset));

                    // 3. Extract AABB (glTF mandates min/max values exist on the POSITION accessor)
                    if (attrib.first == "POSITION" && accessor.minValues.size() == 3 && accessor.maxValues.size() == 3) {
                        glm::vec3 p_min(accessor.minValues[0], accessor.minValues[1], accessor.minValues[2]);
                        glm::vec3 p_max(accessor.maxValues[0], accessor.maxValues[1], accessor.maxValues[2]);
                        result.aabb_min = glm::min(result.aabb_min, p_min);
                        result.aabb_max = glm::max(result.aabb_max, p_max);
                    }
                }

                glBindVertexArray(0); // Unbind VAO
                engine_mesh.primitives.push_back(engine_primitive);
            }
            result.meshes.push_back(engine_mesh);
        }

        // --- STEP 3: Process the Scene Graph ---
        // Find the default scene (fallback to 0 if it's not explicitly set)
        int default_scene_index = model.defaultScene > -1 ? model.defaultScene : 0;

        // Safety check to ensure the file actually has a scene
        if (default_scene_index < model.scenes.size()) {
            const tinygltf::Scene& scene = model.scenes[default_scene_index];
            
            // The root nodes have no parent, so their parent matrix is just the Identity matrix
            glm::mat4 identity_matrix = glm::mat4(1.0f);
            
            // Loop through all the root nodes in the scene and start the recursive walk
            for (int root_node_index : scene.nodes) {
                process_node(model, root_node_index, identity_matrix, result);
            }
        }

        return result;
    }
}
